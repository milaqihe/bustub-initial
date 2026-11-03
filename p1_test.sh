#!/usr/bin/env bash
set -euo pipefail

p1_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
p1_build_dir="${1:-${p1_root}/build}"
p1_cmake="${P1_CMAKE:-cmake}"

# Reuse configured toolchain/Sanitizer settings. Never format source in place.
"$p1_cmake" -S "$p1_root" -B "$p1_build_dir"
"$p1_cmake" --build "$p1_build_dir" --target check-p1 -j "${P1_JOBS:-4}"
