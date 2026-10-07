#!/usr/bin/env python3
"""Aprende la tabla de combate (PoliticaAprendida) por refuerzo en el simulador.

Monte Carlo con exploración ε-greedy: en cada ronda se juegan combates contra
enemigos quietos, errantes y que embisten, en varios entornos; cada acción
tomada en cada estado recibe la recompensa final del combate (+1 si saca al
enemigo, -1 si se cae, 0 si se acaba el tiempo) descontada por el tiempo que
faltaba, y Q(estado, acción) es la media de esos retornos. La tabla parte de
la estrategia escrita a mano.

Al acabar escribe include/estrategia/TablaPolitica.h (la mejor acción de cada estado) y
simulacion/politica_q.txt (la tabla Q, para seguir entrenando con --continuar).

  python3 simulacion/entrenar_politica.py --rondas 12 --n 40
  python3 simulacion/simular.py --param UsarPoliticaAprendida=true   # evaluar
"""
import argparse
import os
import random
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

sys.path.insert(0, os.path.dirname(__file__))
import simular

RAIZ = simular.RAIZ
NE, NA = 64, 8
ACCIONES = ["AtaqueFrontal", "AjusteIzq", "AjusteDer", "CorregirIzq", "CorregirDer",
            "DefensaIzq", "DefensaDer", "Busqueda"]
# El mismo robot y dohyo que la batería de combates (bateria.py)
ROBOT = ["--rpm", "750", "--diam", "0.03", "--masa", "0.34", "--par", "0.6", "--bateria", "1.23",
         "--pala", "0.3", "--pala-rival", "0.3", "--radio", "0.35"]
ENTORNOS = [
    "",
    "--bateria 1.3 --friccion-caja 0.08 --friccion-giro 0.3",
    "--friccion-caja 0.3 --friccion-giro 1.0",
    "--mu 0.7 --bateria 1.4 --inicio-r 0.34",
    "--retardo-piso 40 --mancha 0.006",
    "--pared 0.5 --bateria 1.15",
]
MODOS = ["estatico", "errante", "agresivo", "flanqueo"]
PESO_INICIAL = 5  # la estrategia a mano cuenta como 5 visitas con retorno BONO_MANUAL
BONO_MANUAL = 0.05


def accion_manual(e):
    F, CI, CD, LI, LD = 1, 2, 4, 8, 16
    if e & F:
        if e & CI and not e & CD: return 1
        if e & CD and not e & CI: return 2
        return 0
    if e & CI: return 3
    if e & CD: return 4
    if e & LI: return 5
    if e & LD: return 6
    return 7


def describir(e):
    nombres = ["frontal", "45izq", "45der", "latIzq", "latDer"]
    vistos = [n for i, n in enumerate(nombres) if e >> i & 1]
    return (" + ".join(vistos) or "nada") + (" (último a la der)" if e & 32 else " (último a la izq)")


def jugar(binario, ruta_q, modo, entorno, n, semilla, epsilon):
    cmd = [str(binario), "--modo", modo, "--n", str(n), "--semilla", str(semilla),
           "--q", str(ruta_q), "--epsilon", str(epsilon), *ROBOT, *entorno.split()]
    salida = subprocess.run(cmd, check=True, capture_output=True, text=True).stdout
    visitas, resultados = [], []
    for linea in salida.splitlines():
        if linea.startswith("Q"):
            visitas += [tuple(x.split(":")) for x in linea.split()[1:]]
        elif linea and linea[0].isdigit():
            campos = linea.split(",")
            resultados.append((campos[2] == "1", campos[10] == "1"))  # cayó, ganó
    return visitas, resultados


def guardar_tabla(Q, N, margen=0.0, visitas_min=0):
    # Solo se cambia la acción escrita a mano si la aprendida es mejor por al menos
    # 'margen' y se probó al menos 'visitas_min' veces (si no, puede ser ruido)
    mejor = []
    for e in range(NE):
        m = accion_manual(e)
        b = max(range(NA), key=lambda a: Q[e][a])
        mejor.append(b if Q[e][b] - Q[e][m] >= margen and N[e][b] >= visitas_min else m)
    filas = "\n".join("    " + ", ".join(str(v) for v in mejor[i:i + 8]) + "," for i in range(0, NE, 8))
    cambios = sum(1 for e in range(NE) if mejor[e] != accion_manual(e))
    (RAIZ / "include" / "estrategia" / "TablaPolitica.h").write_text(f"""#ifndef TABLA_POLITICA_H
#define TABLA_POLITICA_H

#include <stdint.h>

// Acción (índice en politica::Acciones) para cada estado de PoliticaAprendida.
// Generada por simulacion/entrenar_politica.py ({cambios} estados distintos de la
// estrategia escrita a mano).
constexpr uint8_t TablaPolitica[64] = {{
{filas}
}};

#endif
""")
    return mejor


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--rondas", type=int, default=12)
    ap.add_argument("--n", type=int, default=30, help="combates por modo y entorno en cada ronda")
    ap.add_argument("--epsilon", type=float, default=0.25)
    ap.add_argument("--continuar", action="store_true", help="partir de simulacion/politica_q.txt")
    ap.add_argument("--margen", type=float, default=0.0, help="mejora mínima de Q para cambiar la acción a mano")
    ap.add_argument("--visitas-min", type=int, default=0)
    a = ap.parse_args()

    ruta_q = RAIZ / "simulacion" / "politica_q.txt"
    ruta_n = RAIZ / "simulacion" / "politica_n.txt"
    if a.continuar and ruta_q.exists():
        Q = [list(map(float, l.split())) for l in ruta_q.read_text().splitlines()]
        N = [list(map(float, l.split())) for l in ruta_n.read_text().splitlines()]
    else:
        Q = [[0.0] * NA for _ in range(NE)]
        N = [[0.0] * NA for _ in range(NE)]
        for e in range(NE):
            Q[e][accion_manual(e)] = BONO_MANUAL
            N[e][accion_manual(e)] = PESO_INICIAL

    simular.BUILD.mkdir(parents=True, exist_ok=True)
    binario = simular.compilar({"UsarPoliticaAprendida": "true"}, ("-DENTRENAMIENTO_POLITICA",))
    tareas = [(m, e) for m in MODOS for e in ENTORNOS]
    for ronda in range(a.rondas):
        epsilon = a.epsilon * (1 - ronda / max(1, a.rondas))  # menos exploración al final
        ruta_q.write_text("\n".join(" ".join(f"{q:.6f}" for q in fila) for fila in Q))
        semilla = 1000 + ronda * 10000
        with ThreadPoolExecutor(os.cpu_count() or 4) as ex:
            res = list(ex.map(lambda t: jugar(binario, ruta_q, t[0], t[1], a.n,
                                              semilla + 997 * tareas.index(t), epsilon), tareas))
        caidas = victorias = total = 0
        for visitas, resultados in res:
            for e, ac, suma, n in visitas:
                e, ac, suma, n = int(e), int(ac), float(suma), int(n)
                Q[e][ac] = (Q[e][ac] * N[e][ac] + suma) / (N[e][ac] + n)
                N[e][ac] += n
            caidas += sum(c for c, _ in resultados)
            victorias += sum(g for _, g in resultados)
            total += len(resultados)
        print(f"ronda {ronda + 1:2d}/{a.rondas}  ε={epsilon:.2f}  caídas {caidas:4d}  victorias {victorias:4d} de {total}")

    ruta_q.write_text("\n".join(" ".join(f"{q:.6f}" for q in fila) for fila in Q))
    ruta_n.write_text("\n".join(" ".join(f"{n:.0f}" for n in fila) for fila in N))
    mejor = guardar_tabla(Q, N, a.margen, a.visitas_min)
    print("\nEstados donde la política aprendida difiere de la escrita a mano:")
    for e in range(NE):
        if mejor[e] != accion_manual(e) and sum(N[e]) > PESO_INICIAL + 50:
            print(f"  {describir(e):<45} {ACCIONES[accion_manual(e)]:>13} -> {ACCIONES[mejor[e]]:<13}"
                  f"  Q {Q[e][accion_manual(e)]:+.3f} -> {Q[e][mejor[e]]:+.3f}  ({int(sum(N[e]))} visitas)")


if __name__ == "__main__":
    main()
