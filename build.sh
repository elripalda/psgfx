#!/usr/bin/env bash
# Builds PSGFX.exe (a single static Windows executable).
#   Linux / WSL:  sudo apt install g++-mingw-w64-x86-64 && ./build.sh
#   Windows:      run from an MSYS2 "UCRT64" or "MINGW64" shell with the mingw-w64 gcc package
# Pass "linux" to build a native Linux binary for development instead: ./build.sh linux
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p build

BLAKE3_FLAGS="-O2 -DBLAKE3_NO_SSE2 -DBLAKE3_NO_SSE41 -DBLAKE3_NO_AVX2 -DBLAKE3_NO_AVX512"
SQLITE_FLAGS="-O2 -DSQLITE_THREADSAFE=1 -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_DEFAULT_MEMSTATUS=0"
SOURCES="src/main.cpp src/ui.cpp src/assets_embed.cpp src/lib/bc7enc.cpp src/lib/bc7decomp.cpp"

if [[ "${1:-}" == "linux" ]]; then
  for f in src/lib/blake3.c src/lib/blake3_dispatch.c src/lib/blake3_portable.c; do
    gcc $BLAKE3_FLAGS -c "$f" -o "build/$(basename "${f%.c}").o"
  done
  [[ -f build/sqlite3.o ]] || gcc $SQLITE_FLAGS -c src/lib/sqlite3.c -o build/sqlite3.o
  g++ -O2 -std=c++17 -pthread $SOURCES build/blake3*.o build/sqlite3.o -o build/psgfx
  echo "Built build/psgfx"
  exit 0
fi

if command -v x86_64-w64-mingw32-g++ >/dev/null; then P=x86_64-w64-mingw32-; else P=""; fi
for f in src/lib/blake3.c src/lib/blake3_dispatch.c src/lib/blake3_portable.c; do
  ${P}gcc $BLAKE3_FLAGS -c "$f" -o "build/$(basename "${f%.c}").o"
done
[[ -f build/sqlite3_win.o ]] || ${P}gcc $SQLITE_FLAGS -c src/lib/sqlite3.c -o build/sqlite3_win.o
(cd src && ${P}windres psgfx.rc -O coff -o ../build/psgfx_res.o)
${P}g++ -O2 -std=c++17 -static -static-libgcc -static-libstdc++ -pthread \
  $SOURCES build/blake3*.o build/sqlite3_win.o build/psgfx_res.o -lws2_32 -lshell32 -o build/PSGFX.exe
${P}strip build/PSGFX.exe
echo "Built build/PSGFX.exe"
