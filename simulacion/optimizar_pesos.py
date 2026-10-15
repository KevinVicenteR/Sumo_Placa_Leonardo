#!/usr/bin/env python3
"""Ajusta los pesos y sesgos de la red de EstrategiaPesos (TablaPesos.h).

Búsqueda por coordenadas: para cada peso y cada sesgo prueba sumarle y restarle
varios pasos dejando el resto fijo y se queda con el cambio que más puntos da
en la batería de combates (misma puntuación que optimizar.py). Como solo
importa qué neurona gana en cada estado, un cambio que no altera ninguna
decisión de ningún estado no se simula.

Los combates de la búsqueda usan unas semillas y la validación final otras.
Al acabar, si los pesos nuevos ganan en la validación, reescribe TablaPesos.h.

  python3 simulacion/optimizar_pesos.py                  # unas horas
  python3 simulacion/optimizar_pesos.py --n 4 --pasadas 1
  python3 simulacion/optimizar_pesos.py --no-escribir     # solo mostrar el resultado
  python3 simulacion/optimizar_pesos.py --entradas Borde,Perd,Emp   # solo esas columnas
"""
import argparse
import os
import re
import sys
import time

sys.path.insert(0, os.path.dirname(__file__))
import simular
import bateria as B
from optimizar import PESO_EMPUJADO, PESO_PROPIA

TABLA = simular.RAIZ / "include" / "estrategia" / "TablaPesos.h"
NEURONAS = ["AtaqueFrontal", "AjusteIzq", "AjusteDer", "CorregirIzq", "CorregirDer",
            "DefensaIzq", "DefensaDer", "Busqueda"]
ENTRADAS = ["Fr", "45I", "45D", "LatI", "LatD", "Lado", "Borde", "Perd", "Emp"]
FILA = r"\{\s*-?\d+(?:,\s*-?\d+){%d}\s*\}" % (len(ENTRADAS) - 1)
PASOS = [-4, -1, 1, 4]
LIMITE = 20


def leer_tabla():
    texto = TABLA.read_text()
    filas = re.findall(FILA, texto)
    pesos = [[int(v) for v in re.findall(r"-?\d+", f)] for f in filas]
    bloque = texto.split("Sesgos[NumNeuronas] = {")[1].split("};")[0]
    sesgos = [int(v) for v in re.findall(r"^\s*(-?\d+),", bloque, re.M)]
    assert len(pesos) == 8 and len(sesgos) == 8
    return texto, pesos, sesgos


def escribir_tabla(texto, pesos, sesgos):
    filas = iter(pesos)
    texto = re.sub(FILA,
                   lambda _: "{ " + ",".join(f"{v:>4}" for v in next(filas)) + " }", texto)
    cab, resto = texto.split("Sesgos[NumNeuronas] = {")
    bloque, fin = resto.split("};", 1)
    valores = iter(sesgos)
    bloque = re.sub(r"\n\s*-?\d+,", lambda m: f"\n    {next(valores):>2},", bloque)
    return cab + "Sesgos[NumNeuronas] = {" + bloque + "};" + fin


def decisiones(pesos, sesgos):
    """Neurona ganadora en cada uno de los estados (igual que RedPesos.cpp,
    con una activación creciente y UmbralActivacion = 0,5: suma >= 0)."""
    tabla = []
    for e in range(1 << len(ENTRADAS)):
        x = [(e >> b) & 1 for b in range(len(ENTRADAS))]
        sumas = [sesgos[n] + sum(xi * w for xi, w in zip(x, pesos[n])) for n in range(8)]
        mejor = max(range(8), key=lambda n: (sumas[n], -n))
        tabla.append(mejor if sumas[mejor] >= 0 else 7)
    return tuple(tabla)


def evaluar(texto, pesos, sesgos, n, semilla):
    binario = simular.compilar({}, (), {"estrategia/TablaPesos.h": escribir_tabla(texto, pesos, sesgos)})
    original = B.ROBOT
    B.ROBOT = original + ["--semilla", str(semilla)]
    try:
        res = B.bateria(binario, n, 30)
    finally:
        B.ROBOT = original
    v, c, total = B.sumar(res, lambda *_: True)
    propias = B.caidas_propias(res)
    puntos = 100 * (v - PESO_EMPUJADO * (c - propias) - PESO_PROPIA * propias) / total
    return puntos, 100 * v / total, 100 * c / total, propias, res


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--n", type=int, default=6, help="combates por casilla durante la búsqueda")
    ap.add_argument("--pasadas", type=int, default=2)
    ap.add_argument("--validar", type=int, default=20, help="combates por casilla en la validación")
    ap.add_argument("--no-escribir", action="store_true", help="no reescribir TablaPesos.h")
    ap.add_argument("--entradas", default=",".join(ENTRADAS),
                    help="columnas a ajustar, p. ej. Borde,Perd,Emp (los sesgos se ajustan siempre)")
    a = ap.parse_args()
    simular.BUILD.mkdir(parents=True, exist_ok=True)

    texto, pesos, sesgos = leer_tabla()
    inicio = ([f[:] for f in pesos], sesgos[:])
    puntos, vic, cai, pro, _ = evaluar(texto, pesos, sesgos, a.n, 1)
    vistas = {decisiones(pesos, sesgos): puntos}
    print(f"inicio: puntos {puntos:.1f}  gana {vic:.1f}%  se cae {cai:.1f}% (solo: {pro})", flush=True)

    # Cada coordenada: (neurona, entrada) para un peso, (neurona, None) para el sesgo
    columnas = [ENTRADAS.index(e) for e in a.entradas.split(",")]
    coordenadas = [(nn, i) for nn in range(8) for i in columnas + [None]]
    for pasada in range(a.pasadas):
        for nn, i in coordenadas:
            for paso in PASOS:
                p2, s2 = [f[:] for f in pesos], sesgos[:]
                if i is None:
                    s2[nn] = max(-LIMITE, min(LIMITE, s2[nn] + paso))
                else:
                    p2[nn][i] = max(-LIMITE, min(LIMITE, p2[nn][i] + paso))
                clave = decisiones(p2, s2)
                if clave in vistas:
                    continue   # mismas decisiones que algo ya simulado
                t = time.time()
                pt, v, c, pr, _ = evaluar(texto, p2, s2, a.n, 1)
                vistas[clave] = pt
                nombre = f"{NEURONAS[nn]}.{'sesgo' if i is None else ENTRADAS[i]}{paso:+d}"
                marca = ""
                if pt > puntos:
                    pesos, sesgos, puntos, marca = p2, s2, pt, "  <- mejor"
                print(f"[{pasada + 1}] {nombre}: puntos {pt:.1f} gana {v:.1f}% cae {c:.1f}% (solo: {pr})"
                      f" ({time.time() - t:.0f} s){marca}", flush=True)
        print(f"\nTras la pasada {pasada + 1}: puntos {puntos:.1f}\n", flush=True)

    # Validación con combates nuevos
    print("=== Validación con combates nuevos ===", flush=True)
    p0, v0, c0, _, _ = evaluar(texto, *inicio, a.validar, 1000)
    p1, v1, c1, _, _ = evaluar(texto, pesos, sesgos, a.validar, 1000)
    print(f"pesos de partida : puntos {p0:.1f}  gana {v0:.1f}%  se cae {c0:.1f}%")
    print(f"pesos ajustados  : puntos {p1:.1f}  gana {v1:.1f}%  se cae {c1:.1f}%")
    print("\nPesos ajustados:")
    for nn in range(8):
        print(f"  {NEURONAS[nn]:<14} {pesos[nn]}  sesgo {sesgos[nn]}")
    if p1 > p0 and not a.no_escribir:
        TABLA.write_text(escribir_tabla(texto, pesos, sesgos))
        print(f"\nEscrito {TABLA.relative_to(simular.RAIZ)}")
    elif p1 <= p0:
        print("\nLos pesos ajustados no mejoran en la validación: TablaPesos.h no se toca")


if __name__ == "__main__":
    main()
