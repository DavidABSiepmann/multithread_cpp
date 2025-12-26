#!/usr/bin/env bash
set -e
# Formata todos os .cpp e .h rastreados pelo git (se existir)
files=$(git ls-files '*.cpp' '*.h' 2>/dev/null || true)
if [ -n "$files" ]; then
  clang-format -i $files
else
  echo "Nenhum arquivo .cpp/.h encontrado para formatar"
fi
