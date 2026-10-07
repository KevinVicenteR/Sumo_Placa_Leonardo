#!/bin/sh
# Genera simulacion/web/dohyo.html: la página interactiva con el firmware actual
# y, para comparar, el de un commit anterior (por defecto ec76e53, la versión
# anterior del sumo X3, rama Placa_Leo_nueva_X3).
#
# El firmware se compila a WebAssembly y wasm2js lo traduce a JavaScript, para
# que la página funcione aunque el navegador no permita WebAssembly.
#
#   simulacion/web/generar_pagina.sh [commit-anterior]
#
# Requiere: brew install llvm lld binaryen
set -e
cd "$(dirname "$0")"
ANTERIOR=${1:-ec76e53}
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
Path("dohyo.html").write_text(pagina)
print("dohyo.html generado")
PY
