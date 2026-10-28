"""Meta del usuario sobre una batería --json: victorias y caídas contra rivales vencibles.

Vencibles = sin el combate sin rival ni el de 500 g, y sin los factores extremos
(todo junto, llantas gastadas, gente alrededor). Las caídas propias, en toda la batería.
"""
import json
import sys

FUERA_RIVAL = {"solo", "pesado 500 g"}
FUERA_FACTOR = {"Todo junto (peor caso)", "Llantas gastadas (las dos)", "Gente u objetos alrededor"}


def meta(d):
    c = [x for x in d["casillas"] if x["rival"] not in FUERA_RIVAL and x["factor"] not in FUERA_FACTOR]
    n = sum(x["combates"] for x in c)
    return (100 * sum(x["victorias"] for x in c) / n, 100 * sum(x["caidas"] for x in c) / n,
            d["total"]["caidas_propias"], d["por_rival"]["solo"]["caidas"], d["total"]["combates"])


if __name__ == "__main__":
    for ruta in sys.argv[1:]:
        g, c, p, s, n = meta(json.load(open(ruta)))
        print(f"{ruta.split('/')[-1][:-5]:<34} vencibles: gana {g:5.1f} %  cae {c:5.1f} %   propias {p:3d}  solo cae {s:3d}  ({n})")
