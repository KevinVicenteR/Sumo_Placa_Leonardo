#!/bin/sh
set -e
cd "$(dirname "$0")"
ANTERIOR=${1:-c2d950b}
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
    lineas = [l for l in ruta.read_text().splitlines()
              if not l.startswith("import ") and not l.startswith("export ")]
    return "(function (env) {\n" + "\n".join(lineas) + "\nreturn retasmFunc;\n})"

pagina = Path("plantilla.html").read_text()
pagina = pagina.replace("__SIM_NUEVO__", fabrica(tmp / "nuevo.js"))
pagina = pagina.replace("__SIM_ANTERIOR__", fabrica(tmp / "anterior.js"))
Path("dohyo.html").write_text(pagina)
print("dohyo.html generado")
PY
