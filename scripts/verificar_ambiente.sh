#!/usr/bin/env bash
set -euo pipefail

printf '== GCC ==\n'
gcc --version | head -n 1
printf '\n== CMake ==\n'
cmake --version | head -n 1
printf '\n== raylib ==\n'
pacman -Q raylib
printf '\n== Box2D ==\n'
pacman -Q box2d
printf '\nAmbiente basico OK.\n'
