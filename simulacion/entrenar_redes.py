#!/usr/bin/env python3
"""Entrena la red de lógica y la de control por neuroevolución en el simulador.

Estrategia evolutiva (como OpenAI-ES, con muestras antitéticas): en cada
generación se prueban parejas de pesos θ ± σ·ε en los mismos combates (mismas
semillas para todos) y θ se mueve hacia las ε que más puntúan. Los pesos son
enteros int8, como en el robot; el simulador los carga con --red sin
recompilar.

Puntuación de un candidato:
  objetivo = % de victorias − 20 × % de caídas en las casillas objetivo (todos
             los rounds en condiciones ideales contra quieto, deambula, embiste
             y flanquea)
  resto    = lo mismo en una muestra de las demás casillas (averías, otros
             rivales), para no empeorarlas
  puntos   = objetivo + PESO_RESTO × resto (ambos con igual peso)
  Cada caída sin contacto reciente añade una penalización extra de 60.

Cada --validar generaciones evalúa θ con combates nuevos (objetivo y batería
entera) y guarda candidatos por caídas propias, caídas globales y victorias.
Filtra primero las restricciones para que un candidato inválido no oculte otro. Al acabar,
solo si confirma la mejora de seguridad en semillas nuevas, reescribe TablaRedLogica.h y TablaRedControl.h.

  python3 simulacion/entrenar_redes.py --generaciones 150
  python3 simulacion/entrenar_redes.py --continuar     # sigue desde redes_mejor.txt
  python3 simulacion/entrenar_redes.py --rumania --muestras-final 30000
"""
import argparse
import math
import os
import random
import sys
import tempfile
import time
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

sys.path.insert(0, os.path.dirname(__file__))
import simular
import bateria as B
import redes as R

RIVALES_OBJETIVO = ["quieto", "deambula", "embiste", "flanquea"]
PESO_RESTO = 1.0
PENALIZACION_CAIDA = 20
PENALIZACION_PROPIA = 60
GUARDADO = Path(__file__).parent / "redes_mejor.txt"
CANDIDATO = Path(__file__).parent / "redes_candidato.txt"


def casillas_objetivo():
    ideal = B.FACTORES[0]
    return [(r, rv, ideal) for r in B.ROUNDS for rv in B.RIVALES if rv[0] in RIVALES_OBJETIVO]


def casillas_resto():
    objetivo = {(r[0], rv[0], f[0]) for r, rv, f in casillas_objetivo()}
    return [(r, rv, f) for f in B.FACTORES for r in B.ROUNDS for rv in B.RIVALES
            if (r[0], rv[0], f[0]) not in objetivo]


def combatir(binario, pesos, casilla, n, semilla):
    """(victorias, caídas, combates) de una casilla con estos pesos."""
    r, rv, f = casilla
    extra = B.ROBOT + ["--dur", "30", "--semilla", str(semilla), "--red", str(pesos)] + r[1] + rv[2] + f[1]
    filas = simular.ejecutar(binario, rv[1], n, extra)
    if len(filas) != n:
        raise RuntimeError(f"Validación incompleta: se esperaban {n} combates y llegaron {len(filas)}")
    return (sum(x["gano"] == "1" for x in filas), sum(x["cayo"] == "1" for x in filas), len(filas),
            sum(x["cayo"] == "1" and x.get("empujado") != "1" for x in filas),
            sum(float(x["t_gano"]) for x in filas if x["gano"] == "1"))


def puntuar(resultados):
    v = sum(x[0] for x in resultados)
    c = sum(x[1] for x in resultados)
    t = sum(x[2] for x in resultados)
    if t == 0:
        raise ValueError("No se pueden puntuar cero combates")
    propias = sum(x[3] if len(x) > 3 else 0 for x in resultados)
    tiempo = sum(x[4] if len(x) > 4 else 30 * x[0] for x in resultados)
    # Solo premia tiempo de victorias; acabar pronto perdiendo nunca puntúa.
    rapidez = max(0, min(v, v - tiempo / 30))
    return (100 * (v + 0.05 * rapidez - PENALIZACION_CAIDA * c - PENALIZACION_PROPIA * propias) / t,
            100 * v / t, 100 * c / t, 100 * propias / t, tiempo / v if v else 30)


class Evaluador:
    def __init__(self, binario, carpeta):
        self.binario = binario
        self.carpeta = Path(carpeta)
        self.contador = 0
        self.ex = ThreadPoolExecutor(os.cpu_count())

    def archivo(self, logica, control):
        self.contador += 1
        ruta = self.carpeta / f"c{self.contador}.red"
        R.guardar_pesos(ruta, logica, control)
        return ruta

    def evaluar_varios(self, candidatos, casillas_obj, casillas_resto, n, semilla):
        """Puntos de cada candidato (todos en los mismos combates)."""
        rutas = [self.archivo(*c) for c in candidatos]
        tareas = [(i, "o", c) for i in range(len(rutas)) for c in casillas_obj]
        tareas += [(i, "r", c) for i in range(len(rutas)) for c in casillas_resto]
        res = list(self.ex.map(lambda t: (t[0], t[1], combatir(self.binario, rutas[t[0]], t[2], n, semilla)), tareas))
        puntos = []
        for i in range(len(rutas)):
            obj = puntuar([x for j, k, x in res if j == i and k == "o"])
            resto = puntuar([x for j, k, x in res if j == i and k == "r"]) if casillas_resto else (0, 0, 0, 0, 30)
            puntos.append((obj[0] + PESO_RESTO * resto[0], obj, resto))
        for ruta in rutas:
            ruta.unlink()
        return puntos

    def validar(self, logica, control, n, semilla):
        """Objetivo y batería entera con combates nuevos."""
        ruta = self.archivo(logica, control)
        todas = [(r, rv, f) for f in B.FACTORES for r in B.ROUNDS for rv in B.RIVALES]
        res = list(self.ex.map(lambda c: (c, combatir(self.binario, ruta, c, n, semilla)), todas))
        ruta.unlink()
        objetivo = {(r[0], rv[0], f[0]) for r, rv, f in casillas_objetivo()}
        obj = puntuar([x for (r, rv, f), x in res if (r[0], rv[0], f[0]) in objetivo])
        total = puntuar([x for _, x in res])
        return obj, total


# "seguridad" (por defecto): primero menos caídas en toda la batería.
# "objetivo": primero 0 caídas y más victorias en las casillas objetivo, sin
# empeorar las caídas ni las caídas propias de la batería entera.
PRIORIDAD = "seguridad"


def prioridad_validacion(obj, total):
    if PRIORIDAD == "objetivo":
        return (-round(obj[2], 1), round(obj[1], 1), -total[3], -total[2], total[1])
    # Primero seguridad en toda la batería; después capacidad de ganar.
    return (-total[3], -total[2], total[1], -(total[4] if len(total) > 4 else 30), -obj[2], obj[1])


def mejora_segura(obj, total, base_obj, base_total):
    if PRIORIDAD == "objetivo":
        return (prioridad_validacion(obj, total) > prioridad_validacion(base_obj, base_total)
                and obj[2] <= base_obj[2] and obj[1] >= base_obj[1]
                and total[2] <= base_total[2] + 0.5 and total[3] <= base_total[3] + 0.1
                and total[1] >= base_total[1] - 0.5)
    return (prioridad_validacion(obj, total) > prioridad_validacion(base_obj, base_total)
            and total[2] <= base_total[2]
            and total[1] >= base_total[1] - 2
            and obj[2] <= base_obj[2])


def elegir_candidato(evaluaciones, obj0, total0, mejor_obj, mejor_total):
    # Filtrar antes de ordenar: un candidato muy conservador que deja de ganar
    # no debe ocultar a otro que sí mejora seguridad y conserva las victorias.
    if PRIORIDAD == "objetivo":
        # Las restricciones de seguridad se comprueban contra la partida; frente
        # al mejor hasta ahora basta con tener más prioridad
        validas = [x for x in evaluaciones
                   if mejora_segura(x[1], x[2], obj0, total0)
                   and prioridad_validacion(x[1], x[2]) > prioridad_validacion(mejor_obj, mejor_total)]
        return max(validas, key=lambda x: prioridad_validacion(x[1], x[2])) if validas else None
    validas = [x for x in evaluaciones
               if mejora_segura(x[1], x[2], obj0, total0)
               and mejora_segura(x[1], x[2], mejor_obj, mejor_total)]
    return max(validas, key=lambda x: prioridad_validacion(x[1], x[2])) if validas else None


PARAMETROS_MODO4 = simular.RAIZ / "include" / "ParametrosModo4.h"


def parametros_con_modos(modos):
    """ParametrosModo4.h con la red de lógica y la de control activas en estos
    modos (1 y/o 3; solo actúan después de la rutina de inicio)."""
    import re
    texto = PARAMETROS_MODO4.read_text()
    for m in modos:
        for nombre in (f"UsarRedLogicaModo{m}", f"UsarRedControlModo{m}"):
            texto, n = re.subn(rf"(constexpr bool {nombre}\s*=\s*)\w+;", r"\g<1>true;", texto)
            if n != 1:
                sys.exit(f"No se encontró {nombre} en ParametrosModo4.h")
    return texto


def redondear(theta):
    return [max(-127, min(127, int(round(v)))) for v in theta]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--generaciones", type=int, default=150)
    ap.add_argument("--parejas", type=int, default=12, help="parejas antitéticas por generación")
    ap.add_argument("--sigma", type=float, default=3.0, help="ruido de exploración (unidades int8)")
    ap.add_argument("--paso", type=float, default=1.5, help="paso de aprendizaje")
    ap.add_argument("--n", type=int, default=4, help="combates por casilla en el entrenamiento")
    ap.add_argument("--resto", type=int, default=48, help="casillas del resto por generación")
    ap.add_argument("--validar", type=int, default=10, help="generaciones entre validaciones")
    ap.add_argument("--n-validar", type=int, default=10)
    ap.add_argument("--n-final", type=int, default=20, help="combates por casilla en la comprobación final")
    ap.add_argument("--continuar", action="store_true", help="partir de redes_mejor.txt")
    ap.add_argument("--no-escribir", action="store_true", help="no reescribir las tablas")
    ap.add_argument("--probar-esquivas", action="store_true", help="evaluar rutas laterales que el sesgo original mantiene inactivas")
    ap.add_argument("--modos-red", default="",
                    help="modos (1 y/o 3) en los que activar las dos redes tras su rutina de inicio, p. ej. 1,3")
    ap.add_argument("--muestras-final", type=int, default=0,
                    help="combates de la comprobación final en total (p. ej. 25000; sustituye a --n-final)")
    ap.add_argument("--guardar-candidatos", default="",
                    help="carpeta donde guardar los pesos de cada candidato validado (formato --red)")
    ap.add_argument("--prioridad", choices=("seguridad", "objetivo"), default="seguridad",
                    help="criterio para aceptar candidatos (ver PRIORIDAD)")
    ap.add_argument("--rumania", action="store_true",
                    help="entrena y valida con las colocaciones de Rumania (bateria.ROUNDS_RUMANIA)")
    a = ap.parse_args()
    if a.rumania:
        B.ROUNDS = B.ROUNDS_RUMANIA
    global PRIORIDAD
    PRIORIDAD = a.prioridad
    casillas_batería = len(B.ROUNDS) * len(B.RIVALES) * len(B.FACTORES)
    if a.muestras_final > 0:
        a.n_final = -(-a.muestras_final // casillas_batería)
    modos = [int(m) for m in a.modos_red.split(",") if m.strip()]
    if any(m not in (1, 3) for m in modos):
        ap.error("--modos-red solo admite 1 y 3 (los modos 2 y 4 ya usan las redes)")
    parametros_modo4 = parametros_con_modos(modos)

    if min(a.generaciones, a.parejas, a.n, a.validar, a.n_validar, a.n_final, a.resto) <= 0:
        ap.error("generaciones, parejas, n, validar, n-validar y resto deben ser positivos")
    if a.sigma <= 0 or a.paso <= 0:
        ap.error("sigma y paso deben ser positivos")

    simular.BUILD.mkdir(parents=True, exist_ok=True)
    activar = {f"UsarRed{red}Modo{m}": "true" for m in modos for red in ("Logica", "Control")}
    binario = simular.compilar(activar, banderas=("-DENTRENAMIENTO_REDES",))
    # Firmware de partida con su selección original de modos: la referencia de
    # la comprobación final cuando se activan modos nuevos
    binario_base = simular.compilar({}, banderas=("-DENTRENAMIENTO_REDES",)) if modos else binario
    print(f"Redes activas también en los modos {modos} tras su rutina de inicio" if modos else
          "Redes con la selección de modos de ParametrosModo4.h", flush=True)
    print(f"Comprobación final: {a.n_final} combates por casilla = {a.n_final * casillas_batería} combates", flush=True)
    nl = R.num_parametros(R.LOGICA)
    checkpoint = CANDIDATO if CANDIDATO.exists() else GUARDADO
    if a.continuar and checkpoint.exists():
        v = [int(x) for x in checkpoint.read_text().split()]
        if len(v) != nl + R.num_parametros(R.CONTROL) or any(abs(x) > 127 for x in v):
            ap.error("El checkpoint no tiene la forma o rango int8 esperado")
        theta = [float(x) for x in v]
    else:
        theta = [float(x) for x in R.leer_tabla(R.TABLA_LOGICA) + R.leer_tabla(R.TABLA_CONTROL)]
    inicio = redondear(theta)
    dim = len(theta)
    rng = random.Random(12345)

    with tempfile.TemporaryDirectory() as carpeta:
        ev = Evaluador(binario, carpeta)
        obj0, total0 = ev.validar(inicio[:nl], inicio[nl:], a.n_validar, 100000)
        print(f"partida: objetivo gana {obj0[1]:.1f}% cae {obj0[2]:.1f}% | "
              f"batería gana {total0[1]:.1f}% cae {total0[2]:.1f}%", flush=True)
        mejor = (prioridad_validacion(obj0, total0), inicio, obj0, total0)
        obj_casillas = casillas_objetivo()
        resto = casillas_resto()

        for g in range(1, a.generaciones + 1):
            t0 = time.time()
            semilla = 1000 + g * 37
            ruidos = [[rng.gauss(0, 1) for _ in range(dim)] for _ in range(a.parejas)]
            candidatos = []
            for e in ruidos:
                for signo in (1, -1):
                    p = redondear([t + signo * a.sigma * x for t, x in zip(theta, e)])
                    candidatos.append((p[:nl], p[nl:]))
            muestra = rng.sample(resto, min(a.resto, len(resto)))
            puntos = ev.evaluar_varios(candidatos, obj_casillas, muestra, a.n, semilla)
            # Rangos centrados (robusto a puntuaciones extremas)
            orden = sorted(range(len(puntos)), key=lambda i: puntos[i][0])
            rango = [0.0] * len(puntos)
            for k, i in enumerate(orden):
                rango[i] = k / (len(puntos) - 1) - 0.5
            grad = [0.0] * dim
            for j, e in enumerate(ruidos):
                peso = rango[2 * j] - rango[2 * j + 1]
                for d in range(dim):
                    grad[d] += peso * e[d]
            theta = [max(-127.0, min(127.0, t + a.paso * gr / a.parejas)) for t, gr in zip(theta, grad)]
            mp = max(puntos, key=lambda x: x[0])
            print(f"[{g}] mejor candidato: objetivo gana {mp[1][1]:.0f}% cae {mp[1][2]:.0f}% | "
                  f"resto gana {mp[2][1]:.0f}% cae {mp[2][2]:.0f}% ({time.time() - t0:.0f} s)", flush=True)

            if g % a.validar == 0 or g == a.generaciones:
                # Comparación sobre las mismas semillas; no confundir azar con mejora.
                propuestas = [redondear(theta), sum(candidatos[max(range(len(puntos)), key=lambda i: puntos[i][0])], [])]
                if a.probar_esquivas and g == min(a.validar, a.generaciones):
                    propuestas += [R.propuesta_esquiva(inicio[:nl], fuerza) + inicio[nl:] for fuerza in (64, 96)]
                evaluaciones = [(p, *ev.validar(p[:nl], p[nl:], a.n_validar, 100000)) for p in propuestas]
                if a.guardar_candidatos:
                    carpeta_c = Path(a.guardar_candidatos)
                    carpeta_c.mkdir(parents=True, exist_ok=True)
                    for k, (pc, oc, tc) in enumerate(evaluaciones):
                        nombre = f"g{g:03d}_{k}_obj{oc[1]:.1f}-{oc[2]:.1f}_bat{tc[1]:.1f}-{tc[2]:.1f}-{tc[3]:.2f}.red"
                        R.guardar_pesos(carpeta_c / nombre, pc[:nl], pc[nl:])
                elegida = elegir_candidato(evaluaciones, obj0, total0, mejor[2], mejor[3])
                p, obj, total = elegida or max(evaluaciones, key=lambda x: prioridad_validacion(x[1], x[2]))
                marca = ""
                # El mejor reduce caídas globales sin perder más de 2 puntos de victorias.
                if elegida is not None:
                    mejor = (prioridad_validacion(obj, total), p, obj, total)
                    theta = [float(x) for x in p]
                    CANDIDATO.write_text(" ".join(map(str, p)) + "\n")
                    marca = "  <- mejor (guardado)"
                print(f"== validación {g}: objetivo gana {obj[1]:.1f}% cae {obj[2]:.1f}% | "
                      f"batería gana {total[1]:.1f}% cae {total[2]:.1f}%{marca}", flush=True)

        # Validación final con combates nuevos: partida frente al mejor
        print("\n=== Validación final ===", flush=True)
        o0, t0_ = ev.validar(inicio[:nl], inicio[nl:], a.n_final, 900000)
        if mejor[1] == inicio:
            o1, t1 = o0, t0_  # Misma red, mismas semillas: no repetir 8400 combates.
        else:
            o1, t1 = ev.validar(mejor[1][:nl], mejor[1][nl:], a.n_final, 900000)
        print(f"partida : objetivo gana {o0[1]:.1f}% cae {o0[2]:.1f}% | batería gana {t0_[1]:.1f}% cae {t0_[2]:.1f}% propias {t0_[3]:.2f}% | victoria media {t0_[4]:.2f}s")
        print(f"entrenada: objetivo gana {o1[1]:.1f}% cae {o1[2]:.1f}% | batería gana {t1[1]:.1f}% cae {t1[2]:.1f}% propias {t1[3]:.2f}% | victoria media {t1[4]:.2f}s")
        segura = mejora_segura(o1, t1, o0, t0_)
        if modos:
            # Activar las redes en más modos solo si también mejora al firmware
            # actual, que no las usa en esos modos
            base = Evaluador(binario_base, carpeta)
            ob, tb = base.validar(inicio[:nl], inicio[nl:], a.n_final, 900000)
            print(f"firmware actual (modos originales): objetivo gana {ob[1]:.1f}% cae {ob[2]:.1f}% | "
                  f"batería gana {tb[1]:.1f}% cae {tb[2]:.1f}% propias {tb[3]:.2f}%")
            segura = segura and mejora_segura(o1, t1, ob, tb)
        if segura and not a.no_escribir:
            GUARDADO.write_text(" ".join(map(str, mejor[1])) + "\n")
            R.escribir_logica(mejor[1][:nl])
            R.escribir_control(mejor[1][nl:])
            print("Escritas TablaRedLogica.h y TablaRedControl.h")
            if modos:
                PARAMETROS_MODO4.write_text(parametros_modo4)
                print(f"Escrito ParametrosModo4.h con las redes activas en los modos {modos}")
        elif segura:
            print("Mejora segura en esta muestra; tablas conservadas por --no-escribir")
        else:
            print("Sin mejora segura confirmada en semillas nuevas: las tablas no se tocan")


if __name__ == "__main__":
    main()
