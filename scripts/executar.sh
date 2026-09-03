#!/usr/bin/env bash
set -euo pipefail

RAIZ="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$RAIZ"

if [[ ! -x build/recoil-gun ]]; then
    echo 'Executavel ainda nao existe. Compilando primeiro...'
    ./scripts/compilar.sh
fi

exec ./build/recoil-gun
