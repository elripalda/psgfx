#!/usr/bin/env bash
# Builds PSGFX.exe (a single static Windows executable).
#   Linux / WSL:  sudo apt install g++-mingw-w64-x86-64 && ./build.sh
#   Windows:      run from an MSYS2 "UCRT64" or "MINGW64" shell with the mingw-w64 gcc package
# Pass "linux" to build a native Linux binary for development instead: ./build.sh linux
# Pass "mac" on a Mac (Xcode command line tools) to build build/PSGFX.app: ./build.sh mac
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p build

# only the portable BLAKE3 is vendored (no blake3_neon.c), so arm64 builds must not pick NEON
BLAKE3_FLAGS="-O2 -DBLAKE3_NO_SSE2 -DBLAKE3_NO_SSE41 -DBLAKE3_NO_AVX2 -DBLAKE3_NO_AVX512 -DBLAKE3_USE_NEON=0"
SQLITE_FLAGS="-O2 -DSQLITE_THREADSAFE=1 -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_DEFAULT_MEMSTATUS=0"
SOURCES="src/main.cpp src/ui.cpp src/assets_embed.cpp src/lib/bc7enc.cpp src/lib/bc7decomp.cpp src/lib/at9/at9_encoder.cpp src/lib/at9/at9_snd0.cpp src/lib/at9/at9_decode.cpp src/lib/at9/at9_tables.cpp"

if [[ "${1:-}" == "mac" ]]; then
  # Universal (Apple Silicon + Intel) app bundle, ad-hoc signed. Its window is src/mac_app.mm.
  MAC="-arch arm64 -arch x86_64 -mmacosx-version-min=11.0"
  APP=build/PSGFX.app
  mkdir -p build/mac
  for f in src/lib/blake3.c src/lib/blake3_dispatch.c src/lib/blake3_portable.c; do
    clang $MAC $BLAKE3_FLAGS -c "$f" -o "build/mac/$(basename "${f%.c}").o"
  done
  [[ -f build/mac/sqlite3.o ]] || clang $MAC $SQLITE_FLAGS -c src/lib/sqlite3.c -o build/mac/sqlite3.o
  clang++ $MAC -O2 -std=c++17 -fobjc-arc -c src/mac_app.mm -o build/mac/mac_app.o
  rm -rf "$APP" build/mac/psgfx.iconset
  mkdir -p "$APP/Contents/MacOS" "$APP/Contents/Resources" build/mac/psgfx.iconset
  clang++ $MAC -O2 -std=c++17 -pthread $SOURCES build/mac/blake3*.o build/mac/sqlite3.o build/mac/mac_app.o \
    -framework Cocoa -framework WebKit -o "$APP/Contents/MacOS/PSGFX"
  VERSION=$(sed -n 's/^#define STUDIO_VERSION "\(.*\)"/\1/p' src/main.cpp)
  sed "s/@VERSION@/$VERSION/g" src/Info.plist > "$APP/Contents/Info.plist"
  for s in 16 32 128 256 512; do   # the app icon, from assets/logo.png (512 px)
    sips -z $s $s assets/logo.png --out "build/mac/psgfx.iconset/icon_${s}x${s}.png" >/dev/null
    if [[ $s -le 256 ]]; then sips -z $((s * 2)) $((s * 2)) assets/logo.png --out "build/mac/psgfx.iconset/icon_${s}x${s}@2x.png" >/dev/null; fi
  done
  iconutil -c icns build/mac/psgfx.iconset -o "$APP/Contents/Resources/psgfx.icns"
  codesign --force --deep -s - "$APP"   # ad-hoc; Gatekeeper still asks on first open (see README)
  echo "Built $APP"
  exit 0
fi
if [[ "${1:-}" == "linux" && "$(uname -s)" == "Darwin" ]]; then echo "On a Mac, run: ./build.sh mac" >&2; exit 1; fi

if [[ "${1:-}" == "linux" ]]; then
  for f in src/lib/blake3.c src/lib/blake3_dispatch.c src/lib/blake3_portable.c; do
    gcc $BLAKE3_FLAGS -c "$f" -o "build/$(basename "${f%.c}").o"
  done
  [[ -f build/sqlite3.o ]] || gcc $SQLITE_FLAGS -c src/lib/sqlite3.c -o build/sqlite3.o
  g++ -O2 -std=c++17 -pthread $SOURCES build/blake3*.o build/sqlite3.o -o build/psgfx
  echo "Built build/psgfx"
  exit 0
fi

if command -v x86_64-w64-mingw32-g++ >/dev/null; then P=x86_64-w64-mingw32-
elif [[ "$(uname -s)" == "Darwin" ]]; then echo "On a Mac, run: ./build.sh mac (or install mingw-w64 to cross-build PSGFX.exe)" >&2; exit 1
else P=""; fi
for f in src/lib/blake3.c src/lib/blake3_dispatch.c src/lib/blake3_portable.c; do
  ${P}gcc $BLAKE3_FLAGS -c "$f" -o "build/$(basename "${f%.c}").o"
done
[[ -f build/sqlite3_win.o ]] || ${P}gcc $SQLITE_FLAGS -c src/lib/sqlite3.c -o build/sqlite3_win.o
(cd src && ${P}windres psgfx.rc -O coff -o ../build/psgfx_res.o)
${P}g++ -O2 -std=c++17 -static -static-libgcc -static-libstdc++ -pthread \
  $SOURCES build/blake3*.o build/sqlite3_win.o build/psgfx_res.o -lws2_32 -lshell32 -o build/PSGFX.exe
${P}strip build/PSGFX.exe
echo "Built build/PSGFX.exe"
