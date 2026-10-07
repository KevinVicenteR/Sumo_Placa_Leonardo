#!/bin/sh
# Compila el firmware real y la física a WebAssembly y genera la página.
#
#   simulacion/web/compilar.sh                 # firmware actual -> sim.wasm
#   simulacion/web/compilar.sh RAIZ salida.wasm   # firmware de otra copia del proyecto
#
# Requiere clang y wasm-ld de LLVM: brew install llvm lld
set -e
cd "$(dirname "$0")"
RAIZ=${1:-../..}
SALIDA=${2:-sim.wasm}
LLVM=/opt/homebrew/opt/llvm/bin
PATH="/opt/homebrew/opt/lld/bin:$PATH"

"$LLVM/clang++" --target=wasm32 -mcpu=mvp -std=c++17 -O2 -ffreestanding -nostdlib \
    -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-c++-static-destructors \
    -Wno-nonportable-include-path \
    -Isupport -I"$RAIZ/include" -I.. \
    $(find "$RAIZ/src" -name "*.cpp") sim_web.cpp \
    -Wl,--no-entry -Wl,--export=__wasm_call_ctors -Wl,--strip-all \
    -o "$SALIDA"

ls -l "$SALIDA"
