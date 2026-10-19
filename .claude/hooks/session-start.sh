#!/bin/bash
# Installs the toolchains needed to build the ROM. Does not build the ROM itself:
# run `make` (matching) or `make modern` when you want a .gba.
set -euo pipefail

if [ "${CLAUDE_CODE_REMOTE:-}" != "true" ]; then
  exit 0
fi

cd "$CLAUDE_PROJECT_DIR"

# arm-none-eabi binutils (both builds) and GCC + newlib (`make modern`)
PKGS="binutils-arm-none-eabi gcc-arm-none-eabi libnewlib-arm-none-eabi libpng-dev"
MISSING=""
for p in $PKGS; do
  dpkg -s "$p" >/dev/null 2>&1 || MISSING="$MISSING $p"
done
if [ -n "$MISSING" ]; then
  echo "Installing:$MISSING"
  apt-get update -qq
  DEBIAN_FRONTEND=noninteractive apt-get install -y -qq $MISSING >/dev/null
fi

# agbcc, the original compiler used by the matching build (`make`)
AGBCC_COMMIT=da598c1d918402c42c0c0d7128ba14567f3175e9
if [ ! -x tools/agbcc/bin/agbcc ]; then
  echo "Building agbcc..."
  WORK=$(mktemp -d)
  git clone -q https://github.com/pret/agbcc "$WORK/agbcc"
  git -C "$WORK/agbcc" checkout -q "$AGBCC_COMMIT"
  (cd "$WORK/agbcc" && ./build.sh >/dev/null 2>&1 && ./install.sh "$CLAUDE_PROJECT_DIR" >/dev/null)
  rm -rf "$WORK"
fi

echo "Toolchains ready. Build with: make modern (or make for the matching ROM)"
