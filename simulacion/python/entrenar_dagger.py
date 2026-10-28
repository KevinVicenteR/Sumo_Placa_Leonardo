#!/usr/bin/env python3
"""DAgger con experto MPC para la tabla de combate (PoliticaAprendida).

El experto es un MPC por muestreo dentro del simulador (--mpc): en cada decisión
prueba las 8 acciones durante el horizonte en procesos hijo y elige la que más
empuja al rival hacia fuera sin acercarnos al borde. DAgger (Ross et al., 2011):
en la iteración i se ejecuta el experto con probabilidad beta_i y si no la tabla
aprendida; las etiquetas del experto en los estados visitados se acumulan y la
tabla es la acción más etiquetada de cada estado.

  python3 -m simulacion.python.entrenar_dagger --datos 50000
  python3 simulacion/bateria.py --rumania --param UsarPoliticaAprendida=true   # evaluar
"""
import argparse
import os
import subprocess
from concurrent.futures import ThreadPoolExecutor

from . import simular
from .bateria import RIVALES, ROUNDS_RUMANIA
from .entrenar_politica import NA, NE, ROBOT, accion_manual, guardar_tabla

RAIZ = simular.RAIZ


def jugar(binario, ruta_q, ronda, rival, semilla, n, beta, horizonte):
    cmd = [str(binario), "--modo", rival[1], "--n", str(n), "--semilla", str(semilla), "--dur", "8",
           "--q", str(ruta_q), "--epsilon", "0", "--mpc", str(horizonte), "--beta", str(beta),
           *ROBOT, *ronda[1], *rival[2]]
    salida = subprocess.run(cmd, check=True, capture_output=True, text=True).stdout
    return [tuple(map(int, x.split(":"))) for l in salida.splitlines() if l.startswith("D") for x in l.split()[1:]]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--datos", type=int, default=50000, help="etiquetas del experto en total")
    ap.add_argument("--iteraciones", type=int, default=4)
    ap.add_argument("--horizonte", type=float, default=0.3, help="s que el MPC simula cada acción")
    a = ap.parse_args()

    conteo = [[0] * NA for _ in range(NE)]
    for e in range(NE):
        conteo[e][accion_manual(e)] = 1  # empate a favor de la acción escrita a mano
    ruta_q = RAIZ / "simulacion" / "resultados" / "dagger_q.txt"
    binario = simular.compilar({"UsarPoliticaAprendida": "true"}, ("-DENTRENAMIENTO_POLITICA",))
    rivales = [r for r in RIVALES if r[0] != "solo"]
    tareas = [(r, v) for r in ROUNDS_RUMANIA for v in rivales]
    meta = a.datos // a.iteraciones
    semilla = 50000
    for i in range(a.iteraciones):
        beta = 1 - i / a.iteraciones  # 1, 0.75, 0.5, 0.25: cada vez manda más el aprendiz
        ruta_q.write_text("\n".join(" ".join(str(c) for c in fila) for fila in conteo))
        nuevos = 0
        while nuevos < meta:
            with ThreadPoolExecutor(os.cpu_count() or 4) as ex:
                lotes = list(ex.map(lambda k: jugar(binario, ruta_q, *tareas[k], semilla + 101 * k, 2, beta,
                                                    a.horizonte), range(len(tareas))))
            semilla += 101 * len(tareas)
            for e, ac in (x for lote in lotes for x in lote):
                conteo[e][ac] += 1
                nuevos += 1
        print(f"iteración {i + 1}/{a.iteraciones}  beta={beta:.2f}  etiquetas {nuevos}", flush=True)

    total = [[c / max(1, sum(fila)) for c in fila] for fila in conteo]
    ruta_q.write_text("\n".join(" ".join(str(c) for c in fila) for fila in conteo))
    mejor = guardar_tabla(total, conteo)
    print(f"{sum(map(sum, conteo)) - NE} etiquetas; {sum(m != accion_manual(e) for e, m in enumerate(mejor))} "
          "estados cambian respecto a la tabla escrita a mano")


if __name__ == "__main__":
    main()
