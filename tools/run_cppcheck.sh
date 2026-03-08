#!/usr/bin/env bash
set -euo pipefail

if ! command -v cppcheck >/dev/null 2>&1; then
  echo "cppcheck is not installed or not on PATH" >&2
  exit 127
fi

profile="${1:-core}"
shift || true

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
suppressions_file="${repo_root}/tools/cppcheck_suppressions.txt"

targets=(
  "${repo_root}/src"
  "${repo_root}/shared"
  "${repo_root}/palette-maker"
  "${repo_root}/tile-maker"
)

common_args=(
  --error-exitcode=1
  --inline-suppr
)

if [[ -f "${suppressions_file}" ]]; then
  common_args+=(--suppressions-list="${suppressions_file}")
fi

case "${profile}" in
  core)
    profile_args=(--enable=warning,style,performance,portability)
    ;;
  strict)
    profile_args=(--enable=all --inconclusive)
    ;;
  *)
    echo "Unknown cppcheck profile: ${profile}" >&2
    echo "Supported profiles: core, strict" >&2
    exit 2
    ;;
esac

cppcheck \
  "${profile_args[@]}" \
  "${common_args[@]}" \
  "$@" \
  "${targets[@]}"
