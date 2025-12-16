#!/bin/sh
set -e

VERSION="$(cat VERSION)"
NAME="LinuxBeaverGEGL-$VERSION"
OUT="$NAME.run"
TITLE="LinuxBeaver GEGL Plugins $VERSION"

meson setup build
DESTDIR=payload ninja -C build install

mkdir -p build/payload
cp VERSION build/payload
cp linux/install.sh build/payload
cp linux/uninstall.sh build/payload

makeself build/payload "$OUT" "$TITLE" ./install.sh

echo "Built installer: $OUT"
