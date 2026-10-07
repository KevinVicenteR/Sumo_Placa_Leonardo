#!/usr/bin/env python3
"""Lista los combates de la batería en los que el robot se cae solo (sin que el
rival lo empuje), para estudiarlos con --tray.

  python3 simulacion/caidas_propias.py --semilla 1000 --n 8 [--param Nombre=valor ...]
"""
import argparse
import os
import sys
from concurrent.futures import ThreadPoolExecutor

sys.path.insert(0, os.path.dirname(__file__))
import simular
import bateria as B
import optimizar as O


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--n", type=int, default=8)
    ap.add_argument("--semilla", type=int, default=1)
    ap.add_argument("--param", action="append", default=[])
    a = ap.parse_args()
    params = dict(p.split("=", 1) for p in a.param)
    binario = simular.compilar(O.ajustar(params))
    casillas = [(r, rv, f) for f in B.FACTORES for r in B.ROUNDS for rv in B.RIVALES]

    def caidas(c):
        r, rv, f = c
        extra = B.ROBOT + ["--semilla", str(a.semilla), "--dur", "30"] + r[1] + rv[2] + f[1]
        filas = simular.ejecutar(binario, rv[1], a.n, extra)
        return [(r[0], rv[0], f[0], x["semilla"], x["t_caida"], x["causa"], " ".join(extra))
                for x in filas if x["cayo"] == "1" and x["empujado"] != "1"]

    with ThreadPoolExecutor(os.cpu_count()) as ex:
        for lista in ex.map(caidas, casillas):
            for x in lista:
                print(" | ".join(x[:6]))
                print("   sim --modo", dict((n, m) for n, m, _ in B.RIVALES)[x[1]], "--n 1 --semilla", x[3], x[6].split("--semilla")[0],
                      x[6].split("--dur 30")[1])


if __name__ == "__main__":
    main()
