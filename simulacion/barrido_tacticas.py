#!/usr/bin/env python3
"""Barrido de las tácticas rápidas de Rumania (TacticaModo1..4 de ParametrosModo4.h).

Cada modo solo se usa en su colocación de Rumania, así que una batería con la
misma táctica k en los cuatro modos mide k en todos a la vez. Para cada táctica
0-7 ejecuta bateria.py --rumania con --muestras combates (mismas semillas en
todas: la comparación es por parejas) y, por modo, elige la que más gana sin
caerse más sola que la rutina de siempre (táctica 0) ni perder en condiciones ideales.

    python3 barrido_tacticas.py --muestras 30000
"""
import argparse
import json
import subprocess
import sys
from pathlib import Path

AQUI = Path(__file__).resolve().parent
NOMBRES = ["rutina de siempre", "carga recta", "contragolpe", "pivote", "giro rápido",
           "斜め / 回り込み", "受け (henka)", "espera y esquiva"]


def correr(k, muestras, carpeta, extra):
    ruta = carpeta / f"tactica_{k}.json"
    if not ruta.exists():
        params = [f"--param=TacticaModo{m}={k}" for m in (1, 2, 3, 4)]
        subprocess.run([sys.executable, str(AQUI / "bateria.py"), "--rumania", "--muestras", str(muestras),
                        "--json", str(ruta)] + params + extra, check=True, stdout=subprocess.DEVNULL)
    return json.loads(ruta.read_text())


def ideal(datos, modo):
    """Victorias y caídas del modo en condiciones ideales (sumando sus casillas)."""
    import bateria
    v = c = n = 0
    for x in datos["casillas"]:
        if x["factor"] == "Ideal" and bateria.MODO_DE_ROUND[x["round"]] == modo and x["rival"] != "solo":
            v, c, n = v + x["victorias"], c + x["caidas"], n + x["combates"]
    return v, c, n


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--muestras", type=int, default=30000)
    ap.add_argument("--carpeta", default=str(AQUI / "resultados" / "tacticas_30000"))
    ap.add_argument("--red", help="pesos de las redes (se pasa a bateria.py)")
    a = ap.parse_args()
    carpeta = Path(a.carpeta)
    carpeta.mkdir(parents=True, exist_ok=True)
    extra = ["--red", a.red] if a.red else []
    sys.path.insert(0, str(AQUI))

    res = {}
    for k in range(len(NOMBRES)):
        res[k] = correr(k, a.muestras, carpeta, extra)
        print(f"táctica {k} ({NOMBRES[k]}): {res[k]['total']}", flush=True)

    eleccion = {}
    for m in (1, 2, 3, 4):
        print(f"\nModo {m}            gana   cae  propias   ideal gana/cae")
        ref = res[0]["por_modo"][str(m)]
        ref_ideal = ideal(res[0], m)
        mejor = 0
        for k in res:
            d = res[k]["por_modo"][str(m)]
            iv, ic, iN = ideal(res[k], m)
            n = d["combates"]
            print(f"  {k} {NOMBRES[k]:<18}{100 * d['victorias'] / n:5.1f} {100 * d['caidas'] / n:5.1f}"
                  f" {d['caidas_propias']:7d}   {100 * iv / iN:5.1f} / {100 * ic / iN:4.1f}")
            b = res[mejor]["por_modo"][str(m)]
            valida = d["caidas_propias"] <= ref["caidas_propias"] and iv >= ref_ideal[0]
            if valida and (d["victorias"], -d["caidas"]) > (b["victorias"], -b["caidas"]):
                mejor = k
        eleccion[m] = mejor
        print(f"  -> modo {m}: táctica {mejor} ({NOMBRES[mejor]})")
    (carpeta / "eleccion.json").write_text(json.dumps(eleccion, indent=1))
    print("\nElección:", eleccion)


if __name__ == "__main__":
    main()
