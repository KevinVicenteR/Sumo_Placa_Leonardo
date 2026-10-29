#!/usr/bin/env python3
"""Diagnóstico de las caídas en la batería de Rumania: por rival, desde qué lado
llegó el rival, qué orden llevaban los motores, si nos hacían retroceder y en
qué momento del combate.

  python3 -m simulacion.python.diagnostico.diagnostico_caidas --muestras 10000 [--red pesos.red] [--json salida.json]
"""
import argparse
import json
import os
from collections import Counter, defaultdict
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

from .. import bateria as B
from .. import simular


def lado(a):
    """Clase del ángulo del rival respecto a nuestro frente (grados)."""
    a = abs(float(a))
    if a > 900:
        return "sin contacto"
    return "frente" if a < 30 else "diagonal" if a < 75 else "costado" if a < 120 else "espalda"


def momento(t):
    t = float(t)
    return "<1 s" if t < 1 else "1-3 s" if t < 3 else "3-10 s" if t < 10 else ">10 s"


def correr(binario, n, red, casilla):
    r, rv, f = casilla
    extra = B.ROBOT + ["--dur", "30"] + red + r[1] + rv[2] + f[1]
    filas = simular.ejecutar(binario, rv[1], n, extra)
    for x in filas:
        x.update(round=r[0], rival=rv[0], factor=f[0])
    return filas


def pct(c, n):
    return f"{100 * c / n:4.0f} %" if n else "   -"


def tabla(titulo, grupos, clave, valores):
    print(f"\n{titulo}")
    print(f"  {'':<16}{'caídas':>8}" + "".join(f"{v:>14}" for v in valores))
    for nombre, filas in grupos.items():
        caidas = [x for x in filas if x["cayo"] == "1"]
        cuenta = Counter(clave(x) for x in caidas)
        print(f"  {nombre:<16}{len(caidas):>8}" + "".join(f"{pct(cuenta[v], len(caidas)):>14}" for v in valores))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--muestras", type=int, default=10000)
    ap.add_argument("--red", help="pesos de las dos redes en vez de las tablas")
    ap.add_argument("--json", help="guarda el resumen aquí")
    a = ap.parse_args()

    casillas = [(r, rv, f) for f in B.FACTORES for r in B.ROUNDS_RUMANIA for rv in B.RIVALES]
    n = -(-a.muestras // len(casillas))
    red = ["--red", str(Path(a.red).resolve())] if a.red else []
    simular.BUILD.mkdir(parents=True, exist_ok=True)
    binario = simular.compilar({})
    print(f"{len(casillas)} casillas x {n} = {len(casillas) * n} combates")
    with ThreadPoolExecutor(max_workers=os.cpu_count()) as ex:
        filas = [x for lote in ex.map(lambda c: correr(binario, n, red, c), casillas) for x in lote]

    por_rival = defaultdict(list)
    for x in filas:
        por_rival[x["rival"]].append(x)
    por_rival["TOTAL"] = filas

    print(f"\n{'rival':<16}{'combates':>9}{'gana':>7}{'cae':>7}{'empujado':>10}{'propia':>8}{'retrocede':>11}{'contacto s':>11}")
    resumen = {}
    for nombre, fs in por_rival.items():
        caidas = [x for x in fs if x["cayo"] == "1"]
        emp = sum(x["empujado"] == "1" for x in caidas)
        atras = sum(float(x["vel_caida"]) < -0.05 for x in caidas)
        cont = sum(float(x["t_contacto"]) for x in fs) / len(fs)
        gana = sum(x["gano"] == "1" for x in fs)
        print(f"{nombre:<16}{len(fs):>9}{pct(gana, len(fs)):>7}{pct(len(caidas), len(fs)):>7}"
              f"{pct(emp, len(caidas)):>10}{pct(len(caidas) - emp, len(caidas)):>8}{pct(atras, len(caidas)):>11}{cont:>11.2f}")
        resumen[nombre] = {
            "combates": len(fs), "victorias": gana, "caidas": len(caidas), "empujado": emp, "retrocede": atras,
            "lado_contacto": Counter(lado(x["ang_contacto"]) for x in caidas),
            "primer_contacto": Counter(lado(x["ang_primer_contacto"]) for x in caidas),
            "orden": Counter(x["causa"] for x in caidas),
            "momento": Counter(momento(x["t_caida"]) for x in caidas),
            "por_round": Counter(x["round"] for x in caidas),
        }

    lados = ["frente", "diagonal", "costado", "espalda", "sin contacto"]
    tabla("Desde dónde estaba el rival en el último contacto antes de caer (% de las caídas)",
          por_rival, lambda x: lado(x["ang_contacto"]), lados)
    tabla("Primer choque de los combates que acaban en caída", por_rival,
          lambda x: lado(x["ang_primer_contacto"]), lados)
    tabla("Orden de motores al caer", por_rival, lambda x: x["causa"],
          ["avance", "curva", "giro", "pivote", "retroceso", "freno", "quieto"])
    tabla("Momento de la caída", por_rival, lambda x: momento(x["t_caida"]), ["<1 s", "1-3 s", "3-10 s", ">10 s"])
    tabla("Caídas por round", por_rival, lambda x: x["round"][:3], ["RU1", "RU2", "RU3", "RU4"])

    # Choques de frente: cuántos ganamos y cuántos perdemos
    print("\nCombates cuyo primer choque fue de frente (|ángulo| < 30°):")
    for nombre, fs in por_rival.items():
        fr = [x for x in fs if lado(x["ang_primer_contacto"]) == "frente"]
        if fr:
            print(f"  {nombre:<16}{len(fr):>7} combates   gana {pct(sum(x['gano'] == '1' for x in fr), len(fr))}"
                  f"   cae {pct(sum(x['cayo'] == '1' for x in fr), len(fr))}")

    if a.json:
        Path(a.json).write_text(json.dumps(resumen, ensure_ascii=False, indent=1))


if __name__ == "__main__":
    main()
