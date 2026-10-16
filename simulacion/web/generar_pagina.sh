#!/bin/sh
# Genera simulacion/web/dohyo.html: la página interactiva con el firmware actual
# y, para comparar, el de un commit anterior (por defecto 10e7a1c, el que se
# llevó al torneo).
#
# El firmware se compila a WebAssembly y wasm2js lo traduce a JavaScript, para
# que la página funcione aunque el navegador no permita WebAssembly.
#
#   simulacion/web/generar_pagina.sh [commit-anterior]
#
# Requiere: brew install llvm lld binaryen
set -e
cd "$(dirname "$0")"
ANTERIOR=${1:-10e7a1c}
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

./compilar.sh ../.. sim.wasm
git -C ../.. archive "$ANTERIOR" src include | tar -x -C "$TMP"
./compilar.sh "$TMP" "$TMP/anterior.wasm"
wasm2js sim.wasm -O2 -o "$TMP/nuevo.js"
wasm2js "$TMP/anterior.wasm" -O2 -o "$TMP/anterior.js"

python3 - "$TMP" <<'PY'
import sys
from pathlib import Path
tmp = Path(sys.argv[1])

def fabrica(ruta):
    # Módulo ES de wasm2js -> función (env) => exports, con memoria propia en cada llamada
    lineas = [l for l in ruta.read_text().splitlines()
              if not l.startswith("import ") and not l.startswith("export ")]
    return "(function (env) {\n" + "\n".join(lineas) + "\nreturn retasmFunc;\n})"

pagina = Path("plantilla.html").read_text()
pagina = pagina.replace("__SIM_NUEVO__", fabrica(tmp / "nuevo.js"))
pagina = pagina.replace("__SIM_ANTERIOR__", fabrica(tmp / "anterior.js"))


def validacion():
    """Tabla por modo con la batería de 25 000 combates (simulacion/resultados/)."""
    import json
    carpeta = Path("../resultados")
    versiones = [(nombre, carpeta / archivo) for nombre, archivo in
                 (("Referencia", "referencia_25000.json"), ("Entrenada", "entrenada_25000.json"))]
    datos = [(n, json.loads(r.read_text())) for n, r in versiones if r.exists()]
    if not datos:
        return '<p class="ayuda">Aún no hay resultados: ejecuta la batería con --json.</p>'

    def fila(titulo, partes):
        celdas = []
        for i, (_, x) in enumerate(partes):
            gana = 100 * x["victorias"] / x["combates"]
            cae = 100 * x["caidas"] / x["combates"]
            clase = ""
            if i > 0:
                g0 = 100 * partes[0][1]["victorias"] / partes[0][1]["combates"]
                c0 = 100 * partes[0][1]["caidas"] / partes[0][1]["combates"]
                p0 = partes[0][1]["caidas_propias"] * x["combates"] / partes[0][1]["combates"]
                propias_peor = x["caidas_propias"] > p0
                clase = (' class="peor"' if cae > c0 or propias_peor else
                         ' class="mejor"' if gana >= g0 and cae <= c0 else "")
            celdas.append(f"<td{clase}>{gana:.1f} %</td><td{clase}>{cae:.1f} %</td><td>{x['caidas_propias']}</td>")
        return f"<tr><td>{titulo}</td>{''.join(celdas)}</tr>"

    cab = "".join(f'<th colspan="3">{n} ({d["total"]["combates"]} combates)</th>' for n, d in datos)
    sub = "".join("<th>gana</th><th>cae</th><th>propias</th>" for _ in datos)
    filas = [fila(f"Modo {m}", [(n, d["por_modo"][m]) for n, d in datos]) for m in ("1", "2", "3", "4")]
    filas.append(fila("<b>Total</b>", [(n, d["total"]) for n, d in datos]))
    return (f'<table class="validacion"><thead><tr><th></th>{cab}</tr><tr><th></th>{sub}</tr></thead>'
            f'<tbody>{"".join(filas)}</tbody></table>')


pagina = pagina.replace("__VALIDACION__", validacion())
Path("dohyo.html").write_text(pagina)
print("dohyo.html generado")
PY
