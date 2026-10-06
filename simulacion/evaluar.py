#!/usr/bin/env python3
"""Evalúa el firmware en todos los entornos (robustos y extremos de barrido.py)
y los 4 escenarios, con el desglose de caídas por causa.

  python3 simulacion/evaluar.py                                  # firmware actual
  python3 simulacion/evaluar.py --n 200                          # más combates
  python3 simulacion/evaluar.py --extra "--retardo-piso 40 --mancha 0.006"   # sensor de piso lento
  python3 simulacion/evaluar.py UsarEstimadorBorde=false         # cambiando parámetros
  python3 simulacion/evaluar.py UsarEstimadorBorde=true --banderas -DODOMETRIA_SIMULADA  # con encoders y giroscopio
"""
import argparse
import os
import sys
from concurrent.futures import ThreadPoolExecutor

sys.path.insert(0, os.path.dirname(__file__))
import barrido
import simular


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("params", nargs="*", help="Nombre=valor de Parametros.h")
    ap.add_argument("--n", type=int, default=100, help="combates por escenario y entorno")
    ap.add_argument("--extra", default="", help="opciones del simulador añadidas a todos los entornos")
    ap.add_argument("--banderas", default="", help="opciones del compilador (p. ej. -DODOMETRIA_SIMULADA)")
    a = ap.parse_args()

    params = dict(p.split("=", 1) for p in a.params)
    simular.BUILD.mkdir(parents=True, exist_ok=True)
    binario = simular.compilar(params, tuple(a.banderas.split()))
    entornos = barrido.ENTORNOS_ROBUSTOS + barrido.ENTORNOS_EXTREMOS

    def correr(ent):
        extra = barrido.ROBOT + ["--dur", "30"] + ent.split() + a.extra.split()
        return ent, [simular.resumir(m, simular.ejecutar(binario, m, a.n, extra)) for m in simular.MODOS]

    propias = duelos = victorias = 0
    vueltas = tirones = 0.0
    print(f"{'entorno':<60} caídas {'/'.join(m[:3] for m in simular.MODOS):<16} victorias")
    with ThreadPoolExecutor(os.cpu_count() or 4) as ex:
        for ent, res in ex.map(correr, entornos):
            c = [r["caidas"] for r in res]
            v = [r["victorias"] for r in res[1:]]
            propias += sum(c[:-1])
            duelos += c[-1]
            victorias += sum(v)
            vueltas += sum(r["vueltas"] for r in res) / len(res) / len(entornos)
            tirones += sum(r["tirones"] for r in res) / len(res) / len(entornos)
            print(f"{(ent or 'nominal')[:60]:<60} {'/'.join(map(str, c)):<23} {'/'.join(map(str, v))}")
            for r in res:
                if r["causas"]:
                    print(f"      {r['modo']}: {r['causas']}")
    total = len(entornos) * len(simular.MODOS) * a.n
    print(f"\n{total} combates: caídas propias (sin enemigo, quieto, errante) {propias}, "
          f"duelos perdidos contra el que embiste {duelos}, victorias {victorias}\n"
          f"suavidad: {vueltas:.1f} vueltas y {tirones:.0f} tirones cada 10 s de combate")


if __name__ == "__main__":
    main()
