#!/usr/bin/env python3
"""Busca las velocidades que más ganan y menos se caen en la batería de combates.

Búsqueda por coordenadas: para cada parámetro prueba varios valores dejando el
resto fijo y se queda con el mejor; repite varias pasadas. La puntuación es
% de victorias − PESO_EMPUJADO × % de caídas empujado por el rival
− PESO_PROPIA × % de caídas propias (el robot se sale solo, lo que nunca
debería pasar), sobre todos los rounds, rivales (también sin rival) y factores
externos (también "todo junto", el peor caso).

Los combates de la búsqueda usan unas semillas y la validación final otras,
para no elegir parámetros que solo funcionan en esos combates concretos.

  python3 simulacion/optimizar.py                       # unos 30-40 min
  python3 simulacion/optimizar.py --n 6 --pasadas 1     # más rápido
"""
import argparse
import os
import sys
import time

sys.path.insert(0, os.path.dirname(__file__))
import simular
import bateria as B

PESO_EMPUJADO = 6
PESO_PROPIA = 30

# Punto de partida: los valores actuales de Parametros.h
INICIO = {}
CANDIDATOS = {
    # Seguridad: estimador de borde (cuándo se fía y qué frenada supone)
    "IncertPosicionConfiable": [0.08, 0.12, 0.16, 0.2],
    "DesaceleracionFreno": [2.0, 2.5, 3.0, 4.0],
    "MargenBorde": [0.04, 0.06, 0.09],
    "VelocidadMinimaEstimador": [60, 80, 100],
    "EstimadorLimitaEmbestida": ["true", "false"],
    "RetrocesoExtraAVelocidadMaxima": [200, 300, 450],
    "VelocidadCercaBorde": [70, 100, 140],
    # Ataque
    "VelocidadAtaque": [130, 160, 200, 255],
    "VelocidadAtaqueDirecto": [0, 200, 255],
    "TiempoEmbestida": [0, 500, 1200, 2500],
    "RampaPwmPorMs": [1, 2, 4],
    "PorcentajeAjuste": [70, 85],
    "VelocidadEmbestidaInicio": [130, 160, 200],
    "VelocidadAtaqueRound12": [160, 200, 255],
    "TiempoEmbestidaInicio": [400, 800, 1500],
    "ResistirEnBorde": ["true", "false"],
    # Giros y búsqueda
    "VelocidadGiroInicio": [150, 180, 220],
    "VelocidadPivoteLateral": [140, 180, 220],
    "VelocidadAvance": [80, 130],
}
# Factores con los que se optimiza: todos, también el peor caso combinado
FACTORES = list(B.FACTORES)


def ajustar(params):
    """Mantiene las reglas del firmware: VelocidadCurva <= VelocidadAtaque."""
    p = dict(params)
    p["VelocidadCurva"] = min(85, int(p.get("VelocidadAtaque", 100)))
    return {k: str(v) for k, v in p.items()}


def evaluar(params, n, semilla, factores=None):
    B.FACTORES = factores or FACTORES
    binario = simular.compilar(ajustar(params))
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
    ap.add_argument("--n", type=int, default=8, help="combates por casilla durante la búsqueda")
    ap.add_argument("--pasadas", type=int, default=2)
    ap.add_argument("--validar", type=int, default=30, help="combates por casilla en la validación")
    a = ap.parse_args()
    simular.BUILD.mkdir(parents=True, exist_ok=True)

    mejor = dict(INICIO)
    puntos, vic, cai, pro, _ = evaluar(mejor, a.n, 1)
    print(f"inicio: puntos {puntos:.1f}  gana {vic:.0f}%  se cae {cai:.1f}% (solo: {pro})", flush=True)
    for pasada in range(a.pasadas):
        for nombre, valores in CANDIDATOS.items():
            for valor in valores:
                if str(mejor.get(nombre)) == str(valor):
                    continue
                prueba = dict(mejor, **{nombre: valor})
                t = time.time()
                p, v, c, pr, _ = evaluar(prueba, a.n, 1)
                marca = ""
                if p > puntos:
                    mejor, puntos, vic, cai, marca = prueba, p, v, c, "  <- mejor"
                print(f"[{pasada + 1}] {nombre}={valor}: puntos {p:.1f} gana {v:.0f}% cae {c:.1f}% (solo: {pr})"
                      f" ({time.time() - t:.0f} s){marca}", flush=True)
        print(f"\nTras la pasada {pasada + 1}: {mejor}\n", flush=True)

    # Validación con combates nuevos y todos los factores (también el peor caso)
    print("=== Validación con combates nuevos ===", flush=True)
    for titulo, params in [("Firmware actual", {}), ("Velocidades optimizadas", mejor)]:
        _, v, c, _, res = evaluar(params, a.validar, 1000, B.FACTORES_TODOS)
        B.FACTORES = B.FACTORES_TODOS
        B.imprimir(res, titulo)
    print("\nParámetros elegidos:")
    for k, v in ajustar(mejor).items():
        print(f"  {k} = {v}")


if __name__ == "__main__":
    B.FACTORES_TODOS = list(B.FACTORES)
    main()
