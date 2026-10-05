#!/bin/sh
set -e
cd "$(dirname "$0")"
RAIZ=${1:-../..}
SALIDA=${2:-sim.wasm}
LLVM=/opt/homebrew/opt/llvm/bin
PATH="/opt/homebrew/opt/lld/bin:$PATH"

"$LLVM/clang++" --target=wasm32 -mcpu=mvp -std=c++17 -O2 -ffreestanding -nostdlib \
    -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-c++-static-destructors \
    -Isupport -I"$RAIZ/include" -I.. \
    "$RAIZ"/src/*.cpp sim_web.cpp \
    -Wl,--no-entry -Wl,--export=__wasm_call_ctors -Wl,--strip-all \
    -o "$SALIDA"

ls -l "$SALIDA"
