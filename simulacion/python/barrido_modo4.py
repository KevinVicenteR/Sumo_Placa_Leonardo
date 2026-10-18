#!/usr/bin/env python3
"""Compara variantes de la táctica del modo 4 solo en las casillas RU4 (de
espaldas) de la batería de Rumania, con todos los rivales y factores.

  python3 -m simulacion.python.barrido_modo4 --n 20 TacticaModo4=6 TacticaModo4=2 "TacticaModo4=6,TiempoRetrocesoLibre=120"
"""
import argparse
import os
from concurrent.futures import ThreadPoolExecutor

from . import bateria as B
from . import simular


def correr(binario, n, casilla):
    r, rv, f = casilla
    filas = simular.ejecutar(binario, rv[1], n, B.ROBOT + ["--dur", "30"] + r[1] + rv[2] + f[1])
    return [(rv[0], f[0], r[0], x["gano"] == "1", x["cayo"] == "1") for x in filas]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--n", type=int, default=20)
    ap.add_argument("variantes", nargs="+", help="Nombre=valor[,Nombre=valor...] de ParametrosModo4.h / Parametros.h")
    a = ap.parse_args()
    simular.BUILD.mkdir(parents=True, exist_ok=True)
    casillas = [(r, rv, f) for f in B.FACTORES for r in B.ROUNDS_RUMANIA if r[0].startswith("RU4") for rv in B.RIVALES]
    for var in a.variantes:
        cambios = dict(p.split("=", 1) for p in var.split(","))
        binario = simular.compilar(cambios)
        with ThreadPoolExecutor(os.cpu_count()) as ex:
            filas = [x for lote in ex.map(lambda c: correr(binario, a.n, c), casillas) for x in lote]
        sin500 = [x for x in filas if x[0] != "pesado 500 g"]
        ideal = [x for x in sin500 if x[1] == "Ideal"]
        n, ni = len(sin500), len(ideal)
        emb = [x for x in ideal if x[0] == "embiste rápido"]
        print(f"{var:<55} gana {100 * sum(x[3] for x in sin500) / n:5.1f} cae {100 * sum(x[4] for x in sin500) / n:5.1f}"
              f" | ideal gana {100 * sum(x[3] for x in ideal) / ni:5.1f} cae {sum(x[4] for x in ideal):3d}/{ni}"
              f" | emb. rápido ideal der {sum(x[4] for x in emb if 'der' in x[2])} izq {sum(x[4] for x in emb if 'izq' in x[2])} de {len(emb) // 2}",
              flush=True)


if __name__ == "__main__":
    main()
