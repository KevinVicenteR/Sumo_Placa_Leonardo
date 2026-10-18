#!/usr/bin/env bash
# Instalación idempotente. Usa herramientas existentes y solo instala lo que falta.
set -euo pipefail
RAIZ="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$RAIZ"
export PATH="$RAIZ/.venv/bin:$PATH"
export PIP_DISABLE_PIP_VERSION_CHECK=1
fallos=0
intentar() { if "$@"; then return 0; else printf 'No se pudo completar: %s\n' "$*" >&2; fallos=1; return 0; fi; }
case "$(uname -s)" in
 Darwin)
  if command -v brew >/dev/null; then
   for pareja in 'python3:python' 'node:node' 'npm:node' 'clang++:llvm' 'git:git'; do
    comando="${pareja%%:*}" paquete="${pareja#*:}"
    command -v "$comando" >/dev/null || intentar brew install "$paquete"
   done
   for paquete in llvm lld; do
    prefijo="$(brew --prefix "$paquete" 2>/dev/null || true)"
    if [[ ! -d "$prefijo/bin" ]]; then intentar brew install "$paquete"; fi
   done
  else echo 'Falta Homebrew: instala https://brew.sh y vuelve a ejecutar este script.' >&2; fallos=1; fi
  ;;
 Linux)
  paquetes=()
  command -v python3 >/dev/null || paquetes+=(python3)
  python3 -c 'import venv, ensurepip' >/dev/null 2>&1 || paquetes+=(python3-venv)
  command -v node >/dev/null || paquetes+=(nodejs)
  command -v npm >/dev/null || paquetes+=(npm)
  command -v clang++ >/dev/null || paquetes+=(clang)
  command -v wasm-ld >/dev/null || paquetes+=(lld)
  command -v c++ >/dev/null || paquetes+=(build-essential)
  command -v git >/dev/null || paquetes+=(git)
  if ((${#paquetes[@]})); then
   if command -v apt-get >/dev/null; then
    privilegio=(); [[ "$EUID" -eq 0 ]] || privilegio=(sudo)
    intentar "${privilegio[@]}" apt-get update
    intentar "${privilegio[@]}" apt-get install -y "${paquetes[@]}"
   else echo 'Se necesita una distribución con apt-get.' >&2; fallos=1; fi
  fi
  ;;
 *) echo 'Sistema compatible: macOS (brew) o Linux (apt).' >&2; exit 1 ;;
esac
if ! command -v pio >/dev/null; then
 intentar python3 -m venv .venv
 if [[ -x .venv/bin/python ]]; then intentar .venv/bin/python -m pip install --timeout 15 --retries 0 'platformio==6.2.0'; fi
fi
if command -v python3 >/dev/null; then
 intentar python3 -c 'import sys; assert sys.version_info >= (3,10), "Se necesita Python >=3.10"'
fi
if command -v node >/dev/null && command -v npm >/dev/null; then
 intentar node -e 'if (+process.versions.node.split(".")[0] < 18) throw Error("Se necesita Node >=18; recomendado 22 LTS")'
 # Las versiones directas están fijadas; si ya coinciden, conserva la instalación.
 if ! node --input-type=commonjs - <<'JS'
const fs = require('node:fs');
try {
const p=JSON.parse(fs.readFileSync('web/package.json','utf8'));
for(const [n,v] of Object.entries({...p.dependencies,...p.devDependencies})) {
 if(JSON.parse(fs.readFileSync(`web/node_modules/${n}/package.json`,'utf8')).version!==v) process.exit(1);
}
} catch { process.exit(1); }
JS
 then intentar npm --prefix web ci --cache "${npm_config_cache:-$RAIZ/.cache/npm}" --fetch-retries 0 --fetch-timeout 15000; fi
else echo 'Node/npm no disponibles; la web queda pendiente.' >&2; fallos=1; fi
# Instala las plataformas y bibliotecas de los entornos declarados, sin compilar ni cargar.
if command -v pio >/dev/null; then
 intentar pio pkg install -e native -e leonardo -e monitoreo -e diagnostico -e prueba_motores
else echo 'PlatformIO no disponible.' >&2; fallos=1; fi
if [[ "$fallos" -ne 0 ]]; then echo 'Instalación incompleta: revisa red, permisos y mensajes anteriores.' >&2; else echo 'Dependencias listas. Ejecuta scripts/correr.sh todo.'; fi
exit "$fallos"
