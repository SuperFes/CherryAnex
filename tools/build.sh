#!/bin/sh
# Builds every qmk.json target in the same container image CI uses.
# The userspace is mounted at its host path so symlinks into it from QMK_HOME resolve.
set -eu

userspace=$(cd "$(dirname "$0")/.." && pwd)
qmk_home=${QMK_HOME:?set QMK_HOME to a qmk_firmware checkout}

exec docker run --rm \
    --user "$(id -u):$(id -g)" -e HOME=/tmp \
    -v "$qmk_home:/qmk_firmware" -e QMK_HOME=/qmk_firmware \
    -v "$userspace:$userspace" -e QMK_USERSPACE="$userspace" \
    -w "$userspace" \
    ghcr.io/qmk/qmk_cli:latest \
    qmk userspace-compile --print-failures "$@"
