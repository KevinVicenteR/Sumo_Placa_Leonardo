#!/usr/bin/env bash
# Compila ambos firmwares con la misma física; no depende de binaryen ni de CDN.
set -euo pipefail
RAIZ="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ANTERIOR="${FIRMWARE_ANTERIOR:-10e7a1c}"
if [[ -z "${LLVM_BIN:-}" ]]; then
 if command -v brew >/dev/null && [[ -x "$(brew --prefix llvm 2>/dev/null)/bin/clang++" ]]; then
  LLVM_BIN="$(brew --prefix llvm)/bin"
 else LLVM_BIN="$(dirname "$(command -v clang++)")"; fi
fi
export PATH="$LLVM_BIN:$PATH"
if command -v brew >/dev/null; then
 LLD_BIN="$(brew --prefix lld 2>/dev/null || true)/bin"
 export PATH="$LLD_BIN:$PATH"
fi
command -v wasm-ld >/dev/null || { echo 'Falta wasm-ld: ejecuta scripts/instalar.sh' >&2; exit 1; }
TEMPORAL="$(mktemp -d)"
trap 'rm -rf "$TEMPORAL"' EXIT
# Acepta tanto la estructura histórica como firmware/.
if git -C "$RAIZ" cat-file -e "$ANTERIOR:firmware" 2>/dev/null; then
 git -C "$RAIZ" archive "$ANTERIOR" firmware | tar -x -C "$TEMPORAL"
else
 git -C "$RAIZ" archive "$ANTERIOR" src include | tar -x -C "$TEMPORAL"
fi
compilar() {
 local proyecto="$1" salida="$2" fuentes=()
 [[ ! -d "$proyecto/firmware" ]] || proyecto="$proyecto/firmware"
 while IFS= read -r archivo; do fuentes+=("$archivo"); done < <(find "$proyecto/src" -name '*.cpp' | LC_ALL=C sort)
 "$LLVM_BIN/clang++" --target=wasm32 -mcpu=mvp -std=c++17 -O2 -ffreestanding -nostdlib \
  -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-c++-static-destructors \
  -Wno-nonportable-include-path -I"$RAIZ/simulacion/web/support" \
  -I"$proyecto/include" -I"$RAIZ/simulacion/nucleo" \
  "${fuentes[@]}" "$RAIZ/simulacion/web/sim_web.cpp" \
  -Wl,--no-entry -Wl,--export=__wasm_call_ctors -Wl,--strip-all -o "$salida"
}
mkdir -p "$RAIZ/web/public"
compilar "$RAIZ" "$RAIZ/web/public/sim-nuevo.wasm"
compilar "$TEMPORAL" "$RAIZ/web/public/sim-anterior.wasm"
printf 'WebAssembly generado: actual y anterior (%s)\n' "$ANTERIOR"
