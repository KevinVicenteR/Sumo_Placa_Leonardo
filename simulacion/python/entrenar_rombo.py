#!/usr/bin/env python3
"""Entrena la búsqueda en rombo (PatronBusqueda = 2) y la protección del borde.

Búsqueda por coordenadas (como optimizar.py): para cada parámetro prueba sus
valores dejando el resto fijo y se queda con el mejor. Cada candidato se mide
con bateria.py --rumania --muestras CRIBA; el ganador y la base se validan al
final con --muestras VALIDACION. Meta (meta_vencibles.py): ganar > 80 % y no
caer contra rivales vencibles, sin caídas propias en toda la batería.

  python3 -m simulacion.python.entrenar_rombo --criba 10000 --validacion 100000
"""
import argparse
import json
import subprocess
import sys

from . import simular
from .meta_vencibles import meta

CAND = simular.RAIZ / "simulacion" / "resultados" / "candidatos"
CANDIDATOS = {
    "VelocidadAvance": [80, 110, 140],
    "TiempoAvanceBusqueda": [600, 1000, 1500],
    "TiempoGiroBusqueda": [100, 150, 200],
    "TiempoGiroRomboObtuso": [250, 300, 400],
    "VelocidadGiroBusqueda": [75, 120],
    "ProtegerTrasLinea": ["false", "true"],
    "ProtegerTrasLineaMs": [300, 800, 1500],
    "Modo1TiempoHenka": [0, 100],
}
INICIO = {"PatronBusqueda": 2, "VelocidadAvance": 80, "TiempoAvanceBusqueda": 1500, "TiempoGiroBusqueda": 150,
          "TiempoGiroRomboObtuso": 300, "VelocidadGiroBusqueda": 75, "ProtegerTrasLinea": "false",
          "ProtegerTrasLineaMs": 800, "Modo1TiempoHenka": 0}


def puntos(d):
    gana, cae, propias, _, total = meta(d)
    return gana - 3 * cae - 50 * 100 * propias / total


def evaluar(nombre, params, muestras, cache={}):
    clave = (tuple(sorted(params.items())), muestras)
    if clave not in cache:
        ruta = CAND / f"{nombre}.json"
        cmd = [sys.executable, str(simular.RAIZ / "simulacion" / "bateria.py"), "--rumania", "--muestras",
               str(muestras), "--json", str(ruta)] + [x for k, v in params.items() for x in ("--param", f"{k}={v}")]
        subprocess.run(cmd, check=True, capture_output=True)
        cache[clave] = json.loads(ruta.read_text())
    return cache[clave]


def informe(nombre, d):
    gana, cae, propias, solo, total = meta(d)
    print(f"{nombre:<28} puntos {puntos(d):7.2f}  vencibles gana {gana:5.1f} % cae {cae:5.1f} %  "
          f"propias {propias} (solo {solo}) de {total}", flush=True)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--criba", type=int, default=10000, help="combates por candidato en la búsqueda")
    ap.add_argument("--validacion", type=int, default=100000, help="combates del ganador y de la base al final")
    ap.add_argument("--pasadas", type=int, default=2)
    a = ap.parse_args()

    CAND.mkdir(parents=True, exist_ok=True)
    mejor = dict(INICIO)
    mejor_d = evaluar("rombo_inicio", mejor, a.criba)
    informe("rombo inicial", mejor_d)
    for pasada in range(a.pasadas):
        cambio = False
        for nombre, valores in CANDIDATOS.items():
            for v in valores:
                if str(v) == str(mejor[nombre]):
                    continue
                prueba = dict(mejor, **{nombre: v})
                d = evaluar("rombo_prueba", prueba, a.criba)
                informe(f"  {nombre}={v}", d)
                if puntos(d) > puntos(mejor_d):
                    mejor, mejor_d, cambio = prueba, d, True
        print(f"pasada {pasada + 1}: {mejor}", flush=True)
        if not cambio:
            break

    print(f"\nValidación con {a.validacion} combates:", flush=True)
    informe("base (firmware actual)", evaluar("validacion_base", {}, a.validacion))
    informe("rombo entrenado", evaluar("validacion_rombo", mejor, a.validacion))
    (CAND / "rombo_mejor.txt").write_text(" ".join(f"--param {k}={v}" for k, v in mejor.items()) + "\n")


if __name__ == "__main__":
    main()
