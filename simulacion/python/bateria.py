#!/usr/bin/env python3
"""Batería de combates: todos los rounds x rivales x factores externos.

Simula cada posición de salida del reglamento (con los interruptores DIP que le
corresponden) contra un rival quieto, uno que deambula y uno que embiste, en
condiciones ideales y con cada perturbación por separado: batería, desgaste de
llantas, polvo, sensores lentos, reflejos, infrarrojo del rival, cortes en la
señal de START... y todas juntas.

  python3 -m simulacion.python.bateria                    # 20 combates por casilla
  python3 -m simulacion.python.bateria --n 50
  python3 -m simulacion.python.bateria --param UsarPoliticaAprendida=true
  python3 -m simulacion.python.bateria --comparar UsarFiltroBayesRival=true
"""
import argparse
from pathlib import Path
import os
import sys
from concurrent.futures import ThreadPoolExecutor

from . import simular

# Robot real: motores y ruedas JSumo (750 rpm, 3 cm), 340 g con la base de metal,
# LiPo 2S (7,4 V) con motores de 6 V (batería 1,23) y pala delantera que le
# quita al rival un 30 % de agarre (valor supuesto: no se ha medido). Dohyo negro
# con borde blanco; fuera del dohyo (elevado) el sensor de piso no ve nada y lee
# oscuro, como el negro: el caso más difícil. El dohyo mide 77 cm de diámetro (línea de 2,5 cm incluida).
ROBOT = ["--rpm", "750", "--diam", "0.03", "--masa", "0.34", "--par", "0.6", "--mu", "0.9",
         "--bateria", "1.23", "--pala", "0.3", "--radio", "0.385",
         # Ruedas atrás (como en el video): el robot gira sobre el eje trasero; el
         # cuerpo empieza 3,3 cm por delante, los sensores de piso van a 7,8 cm
         # y el centro de masa se supone a 2 cm
         "--centro-delante", "0.033", "--sensor-x", "0.078", "--cdm-delante", "0.02",
         # "--vuelco-pala", "1" haría volcar el robot con la pala entera fuera del
         # dohyo; sin calibrar con el robot real sale demasiado pesimista
         ]
# Los rivales que atacan también llevan pala (como el del video)
PALA_RIVAL = ["--pala-rival", "0.3"]

# Posición de salida de cada round y los DIP que hay que poner
# (DIP1 = bit 0, DIP2 = bit 1, DIP3 = bit 2: lado del giro, ON = izquierda)
ROUNDS = [
    ("R1 espalda con espalda", ["--salida", "espalda", "--dip", "0"]),
    ("R2 lado, rival a la der", ["--salida", "lado", "--rival-lado", "der", "--dip", "1"]),
    ("R2 lado, rival a la izq", ["--salida", "lado", "--rival-lado", "izq", "--dip", "5"]),
    ("R3 enfrentados", ["--salida", "frente", "--dip", "2"]),
    ("Salida libre sin rutina", ["--dip", "3"]),
]

# Rumania (RoboChallenge): cada robot detrás de su línea de salida, dentro de su
# franja. Con 10 x 10 cm solo caben colocaciones rectas, así que cada modo es una
# táctica para una colocación: modos 1 y 2 de frente (dos tácticas distintas),
# modo 3 de lado, modo 4 de espaldas. DIP3 OFF = rival / giro a la derecha.
def _rumania(angulo, lado, dip):
    return ["--salida", "rumania", "--angulo-rumania", str(angulo), "--rival-lado", lado, "--dip", str(dip)]


ROUNDS_RUMANIA = [
    ("RU1 de frente", _rumania(0, "der", 0)),
    ("RU2 de frente, DIP3 der", _rumania(0, "der", 1)),
    ("RU2 de frente, DIP3 izq", _rumania(0, "izq", 5)),
    ("RU3 de lado, rival der", _rumania(90, "der", 2)),
    ("RU3 de lado, rival izq", _rumania(90, "izq", 6)),
    ("RU4 de espaldas, gira der", _rumania(180, "der", 3)),
    ("RU4 de espaldas, gira izq", _rumania(180, "izq", 7)),
]

# (nombre, modo del simulador, opciones). "solo" = sin rival: nunca debería caerse
RIVALES = [
    ("solo", "ninguno", []),
    ("quieto", "estatico", []),
    ("deambula", "errante", []),
    ("embiste", "agresivo", PALA_RIVAL),
    ("embiste rápido", "agresivo", ["--vel-agresivo", "0.9", "--agarre-enemigo", "0.9"] + PALA_RIVAL),
    ("pesado 500 g", "agresivo", ["--masa-enemigo", "0.5", "--agarre-enemigo", "0.9"] + PALA_RIVAL),
    ("flanquea", "flanqueo", PALA_RIVAL),
    # Estilos de torneo (informe de Codex): gira y carga, espera y embiste, a
    # pulsos, y se aparta de lado cuando cargamos
    ("tornado", "tornado", PALA_RIVAL),
    ("contragolpe", "contragolpe", PALA_RIVAL),
    ("woodpecker", "woodpecker", PALA_RIVAL),
    ("esquivador", "esquivador", PALA_RIVAL),
]

FACTORES = [
    ("Ideal", []),
    ("LiPo casi descargada (6,7 V)", ["--bateria", "1.12"]),
    ("LiPo recién cargada (8,4 V)", ["--bateria", "1.4"]),
    ("Llantas gastadas (las dos)", ["--desgaste-izq", "0.4", "--desgaste-der", "0.4"]),
    ("Llanta izquierda gastada", ["--desgaste-izq", "0.5"]),
    # El polvo quita agarre a los dos robots (el rival tiene 0,8 de serie)
    ("Dohyo con polvo (poco agarre)", ["--mu", "0.6", "--agarre-enemigo", "0.55"]),
    ("Sensor de piso lento (40 ms)", ["--retardo-piso", "40"]),
    ("Gente u objetos alrededor", ["--pared", "0.5"]),
    ("Reflejos (detecciones falsas)", ["--fantasmas", "2"]),
    ("IR del rival (sensores cegados)", ["--saturacion", "0.3"]),
    ("Cortes en la señal de START", ["--cortes-arranque", "0.5"]),
    ("Todo junto (peor caso)", ["--bateria", "1.12", "--desgaste-izq", "0.5", "--desgaste-der", "0.3",
                                "--mu", "0.7", "--agarre-enemigo", "0.65", "--retardo-piso", "30", "--pared", "0.5",
                                "--fantasmas", "1", "--saturacion", "0.2", "--cortes-arranque", "0.3"]),
]


# Pesos de las redes para el simulador (--red), si se pasan con --red
RED = []


def correr(binario, n, dur, ronda, rival, factor):
    """Una casilla de la tabla: n combates. Devuelve (victorias, caídas, n, caídas propias)."""
    extra = ROBOT + ["--dur", str(dur)] + RED + ronda[1] + rival[2] + factor[1]
    filas = simular.ejecutar(binario, rival[1], n, extra)
    victorias = sum(1 for f in filas if f["gano"] == "1")
    caidas = sum(1 for f in filas if f["cayo"] == "1")
    # Caída propia: el robot se sale sin que el rival lo esté empujando
    propias = sum(1 for f in filas if f["cayo"] == "1" and f["empujado"] != "1")
    return victorias, caidas, len(filas), propias


def bateria(binario, n, dur):
    """Ejecuta todas las casillas en paralelo. Devuelve {(ronda, rival, factor): (v, c, n)}."""
    casillas = [(r, rv, f) for f in FACTORES for r in ROUNDS for rv in RIVALES]
    with ThreadPoolExecutor(max_workers=os.cpu_count()) as ex:
        resultados = list(ex.map(lambda c: correr(binario, n, dur, *c), casillas))
    return {(r[0], rv[0], f[0]): res for (r, rv, f), res in zip(casillas, resultados)}


def sumar(res, filtro):
    v = c = n = 0
    for clave, (vi, ca, ni, _) in res.items():
        if filtro(*clave):
            v, c, n = v + vi, c + ca, n + ni
    return v, c, n


def caidas_propias(res, filtro=lambda *_: True):
    return sum(r[3] for clave, r in res.items() if filtro(*clave))


def pct(x, n):
    return 100 * x / n if n else 0


def imprimir(res, titulo):
    print(f"\n=== {titulo} ===")
    print("\nPor factor externo (todos los rounds y rivales):")
    print(f"  {'factor':<34}{'gana':>7}{'se cae':>8}{'(solo)':>8}{'empata':>8}")
    for nombre, _ in FACTORES:
        filtro = lambda r, rv, f: f == nombre
        v, c, n = sumar(res, filtro)
        p = caidas_propias(res, filtro)
        print(f"  {nombre:<34}{pct(v, n):>6.0f}%{pct(c, n):>7.0f}%{pct(p, n):>7.0f}%{pct(n - v - c, n):>7.0f}%")

    print("\nPor rival (todos los rounds y factores):")
    print(f"  {'rival':<18}{'gana':>7}{'se cae':>8}{'(solo)':>8}")
    for rv in RIVALES:
        filtro = lambda r, x, f: x == rv[0]
        v, c, n = sumar(res, filtro)
        p = caidas_propias(res, filtro)
        print(f"  {rv[0]:<18}{pct(v, n):>6.0f}%{pct(c, n):>7.0f}%{pct(p, n):>7.0f}%")

    print("\nPor round y rival (condiciones ideales | peor caso), % de victorias / % de caídas:")
    print("  (sin rival no se puede ganar: solo cuenta que no se caiga)")
    print(f"  {'round':<26}" + "".join(f"{rv[0]:>18}" for rv in RIVALES))
    for ronda, _ in ROUNDS:
        celdas = []
        for rv in RIVALES:
            vi, ci, ni, _ = res[(ronda, rv[0], "Ideal")]
            vp, cp, np_, _ = res[(ronda, rv[0], "Todo junto (peor caso)")]
            celdas.append(f"{pct(vi, ni):3.0f}/{pct(ci, ni):<3.0f}|{pct(vp, np_):3.0f}/{pct(cp, np_):<3.0f}")
        print(f"  {ronda:<26}" + "".join(f"{c:>18}" for c in celdas))

    v, c, n = sumar(res, lambda *_: True)
    p = caidas_propias(res)
    print(f"\nTotal: {n} combates  gana {pct(v, n):.0f} %  se cae {pct(c, n):.0f} %"
          f" (sin que lo empujen: {p})  empata {pct(n - v - c, n):.0f} %")


# Modo del robot (DIP1 y DIP2) que usa cada round
MODO_DE_ROUND = {"R1 espalda con espalda": 1, "R2 lado, rival a la der": 2, "R2 lado, rival a la izq": 2,
                 "R3 enfrentados": 3, "Salida libre sin rutina": 4}
MODO_DE_ROUND.update({nombre: int(nombre[2]) for nombre, _ in ROUNDS_RUMANIA})


def guardar_json(res, ruta, extra=None):
    """Resultado de cada casilla, más totales por modo, rival y factor."""
    import json
    from pathlib import Path

    def total(filtro):
        v, c, n = sumar(res, filtro)
        return {"combates": n, "victorias": v, "caidas": c, "caidas_propias": caidas_propias(res, filtro)}

    datos = dict(extra or {})
    datos["total"] = total(lambda *_: True)
    datos["por_modo"] = {str(m): total(lambda r, rv, f, m=m: MODO_DE_ROUND[r] == m) for m in (1, 2, 3, 4)}
    datos["por_rival"] = {rv[0]: total(lambda r, x, f, k=rv[0]: x == k) for rv in RIVALES}
    datos["por_factor"] = {f[0]: total(lambda r, rv, x, k=f[0]: x == k) for f in FACTORES}
    datos["casillas"] = [{"round": r, "rival": rv, "factor": f, "victorias": x[0], "caidas": x[1],
                          "combates": x[2], "caidas_propias": x[3]} for (r, rv, f), x in res.items()]
    Path(ruta).parent.mkdir(parents=True, exist_ok=True)
    Path(ruta).write_text(json.dumps(datos, ensure_ascii=False, indent=1))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--n", type=int, default=20, help="combates por casilla")
    ap.add_argument("--muestras", type=int, default=0,
                    help="combates en total como mínimo (se redondea hacia arriba a un número igual por casilla; sustituye a --n)")
    ap.add_argument("--json", help="guarda el resultado de cada casilla en este archivo")
    ap.add_argument("--rumania", action="store_true", help="colocaciones de Rumania (ROUNDS_RUMANIA)")
    ap.add_argument("--red", help="pesos de las dos redes (formato de entrenar_redes.py) en vez de las tablas")
    ap.add_argument("--dur", type=float, default=30, help="s de combate")
    ap.add_argument("--param", action="append", default=[], help="Nombre=valor de Parametros.h")
    ap.add_argument("--comparar", action="append", default=[],
                    help="Nombre=valor: repite la batería con este cambio y muestra la diferencia")
    a = ap.parse_args()

    simular.BUILD.mkdir(parents=True, exist_ok=True)
    base = dict(p.split("=", 1) for p in a.param)
    if a.red:
        RED[:] = ["--red", str(Path(a.red).resolve())]
    if a.rumania:
        global ROUNDS
        ROUNDS = ROUNDS_RUMANIA
    casillas = len(ROUNDS) * len(RIVALES) * len(FACTORES)
    if a.muestras > 0:
        a.n = -(-a.muestras // casillas)
    total = casillas * a.n
    print(f"{len(ROUNDS)} rounds x {len(RIVALES)} rivales x {len(FACTORES)} factores x {a.n} = {total} combates")
    res = bateria(simular.compilar(base), a.n, a.dur)
    imprimir(res, "Firmware actual" + (f" {base}" if base else ""))
    if a.json:
        guardar_json(res, a.json, {"combates_por_casilla": a.n, "parametros": base})

    if a.comparar:
        cambio = dict(base, **dict(p.split("=", 1) for p in a.comparar))
        res2 = bateria(simular.compilar(cambio), a.n, a.dur)
        imprimir(res2, f"Con {a.comparar}")
        print("\nDiferencia por factor (puntos de % de victorias / de caídas):")
        for nombre, _ in FACTORES:
            v1, c1, n1 = sumar(res, lambda r, rv, f: f == nombre)
            v2, c2, n2 = sumar(res2, lambda r, rv, f: f == nombre)
            print(f"  {nombre:<34}{pct(v2, n2) - pct(v1, n1):>+6.0f}{pct(c2, n2) - pct(c1, n1):>+7.0f}")


if __name__ == "__main__":
    main()
