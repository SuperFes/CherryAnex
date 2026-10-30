#!/bin/sh
# Builds every qmk.json target in the same container image CI uses.
# Like CI, each keyboards/* folder is placed into the QMK tree, since QMK
# userspace only overlays keymaps.
set -eu

userspace=$(cd "$(dirname "$0")/.." && pwd)
qmk_home=${QMK_HOME:?set QMK_HOME to a qmk_firmware checkout}

keyboard_mounts=""
for dir in "$userspace"/keyboards/*/; do
    name=$(basename "$dir")
    keyboard_mounts="$keyboard_mounts -v $userspace/keyboards/$name:/qmk_firmware/keyboards/$name"
done

exec docker run --rm \
    --user "$(id -u):$(id -g)" -e HOME=/tmp \
    -v "$qmk_home:/qmk_firmware" -e QMK_HOME=/qmk_firmware \
    $keyboard_mounts \
    -v "$userspace:/qmk_userspace" -e QMK_USERSPACE=/qmk_userspace \
    -w /qmk_userspace \
    ghcr.io/qmk/qmk_cli:latest \
    qmk userspace-compile --print-failures "$@"
