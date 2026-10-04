#!/usr/bin/env python3
"""Compila el firmware con el Arduino simulado y ejecuta lotes de combates.

Ejemplos:
  python3 simulacion/simular.py                       # resumen de los 3 escenarios
  python3 simulacion/simular.py --vmax 1.2 --n 300
  python3 simulacion/simular.py --param TiempoRetroceso=150 --param VelocidadAvance=120
  python3 simulacion/simular.py --tray simulacion/salida  # guarda trayectorias de ejemplo
"""

import argparse
import csv
import hashlib
import io
import re
import shutil
import statistics
import subprocess
import sys
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent
BUILD = RAIZ / "simulacion" / "build"
FUENTES = sorted((RAIZ / "src").glob("*.cpp")) + [RAIZ / "simulacion" / "sim.cpp"]
MODOS = ["ninguno", "estatico", "errante"]


def compilar(params: dict) -> Path:
    """Copia include/ aplicando los cambios a Parametros.H y compila."""
    clave = hashlib.sha1(repr(sorted(params.items())).encode()).hexdigest()[:10]
    inc = BUILD / f"include_{clave}"
    binario = BUILD / f"sim_{clave}"
    if inc.exists():
        shutil.rmtree(inc)
    shutil.copytree(RAIZ / "include", inc)

    ruta = inc / "Parametros.H"
    texto = ruta.read_text()
    for nombre, valor in params.items():
        texto, n = re.subn(rf"(\b{nombre}\s*=\s*)[^;]+;", rf"\g<1>{valor};", texto)
        if n == 0:
            sys.exit(f"Parámetro desconocido: {nombre}")
    ruta.write_text(texto)

    cmd = ["c++", "-std=c++17", "-O2", "-w",
           f"-I{inc}", f"-I{RAIZ / 'simulacion' / 'support'}",
           *map(str, FUENTES), "-o", str(binario)]
    subprocess.run(cmd, check=True)
    return binario


def ejecutar(binario: Path, modo: str, n: int, extra: list, tray: Path | None = None):
    cmd = [str(binario), "--modo", modo, "--n", str(n), *extra]
    if tray:
        cmd += ["--tray", str(tray)]
    salida = subprocess.run(cmd, check=True, capture_output=True, text=True).stdout
    return list(csv.DictReader(io.StringIO(salida)))


def resumir(modo: str, filas: list) -> dict:
    caidas = [f for f in filas if f["cayo"] == "1"]
    margen = [float(f["margen_min_cm"]) for f in filas]
    salida = [float(f["max_salida_cuerpo_cm"]) for f in filas]
    vistos = [float(f["t_primer_frontal"]) for f in filas if float(f["t_primer_frontal"]) >= 0]
    return {
        "modo": modo,
        "n": len(filas),
        "caidas": len(caidas),
        "margen_min": min(margen),
        "margen_p10": sorted(margen)[len(margen) // 10],
        "salida_max": max(salida),
        "t_ver_mediana": statistics.median(vistos) if vistos else float("nan"),
        "frac_frontal": statistics.mean(float(f["frac_frontal"]) for f in filas),
        "semillas_caida": [int(f["semilla"]) for f in caidas][:8],
    }


def imprimir(resumenes: list):
    print(f"{'escenario':<10}{'caídas':>10}{'margen mín':>12}{'margen p10':>12}"
          f"{'sobresale':>11}{'ve enemigo':>12}{'% de frente':>13}")
    for r in resumenes:
        print(f"{r['modo']:<10}{r['caidas']:>5}/{r['n']:<4}{r['margen_min']:>9.1f} cm"
              f"{r['margen_p10']:>9.1f} cm{r['salida_max']:>8.1f} cm"
              f"{r['t_ver_mediana']:>10.2f} s{100 * r['frac_frontal']:>11.0f} %")
        if r["semillas_caida"]:
            print(f"{'':<10}semillas con caída: {r['semillas_caida']}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--n", type=int, default=200, help="corridas por escenario")
    ap.add_argument("--dur", type=float, default=30)
    ap.add_argument("--vmax", type=float, default=0.8)
    ap.add_argument("--mu", type=float, default=0.9)
    ap.add_argument("--tau", type=float, default=0.05)
    ap.add_argument("--rango", type=float, default=0.40)
    ap.add_argument("--rpm", type=float, default=750, help="rpm en vacío (0 = modelo simple con --vmax)")
    ap.add_argument("--diam", type=float, default=0.03, help="diámetro de rueda en m")
    ap.add_argument("--masa", type=float, default=0.3, help="kg")
    ap.add_argument("--par", type=float, default=0.6, help="par de bloqueo por motor en kg·cm")
    ap.add_argument("--modos", default=",".join(MODOS))
    ap.add_argument("--param", action="append", default=[], help="Nombre=valor de Parametros.H")
    ap.add_argument("--tray", type=Path, help="carpeta donde guardar trayectorias de ejemplo")
    a, otros = ap.parse_known_args()

    params = dict(p.split("=", 1) for p in a.param)
    BUILD.mkdir(parents=True, exist_ok=True)
    binario = compilar(params)
    extra = ["--dur", str(a.dur), "--vmax", str(a.vmax), "--mu", str(a.mu),
             "--tau", str(a.tau), "--rango", str(a.rango)]
    # Opciones de realismo (--bateria, --pared, --adc-*, --friccion-*...) pasan tal cual al simulador
    extra += [x for o in otros for x in o.split("=", 1)]
    if a.rpm > 0:
        extra += ["--rpm", str(a.rpm), "--diam", str(a.diam), "--masa", str(a.masa), "--par", str(a.par)]
        v0 = a.rpm / 60 * 3.14159 * a.diam
        print(f"Robot: {a.rpm:.0f} rpm  rueda Ø{a.diam * 100:.1f} cm (→ {v0:.2f} m/s en vacío)  "
              f"{a.masa * 1000:.0f} g  par {a.par} kg·cm  mu={a.mu}  "
              f"| {a.n} combates de {a.dur:.0f} s por escenario")
    else:
        print(f"Robot: vmax={a.vmax} m/s  mu={a.mu}  tau={a.tau}s  rango={a.rango} m  "
              f"| {a.n} combates de {a.dur:.0f} s por escenario")
    if otros:
        print("Entorno:", " ".join(otros))
    if params:
        print("Parámetros modificados:", params)
    resumenes = []
    for modo in a.modos.split(","):
        tray = None
        if a.tray:
            a.tray.mkdir(parents=True, exist_ok=True)
            tray = a.tray / f"tray_{modo}.csv"
        resumenes.append(resumir(modo, ejecutar(binario, modo, a.n, extra, tray)))
    imprimir(resumenes)


if __name__ == "__main__":
    main()
