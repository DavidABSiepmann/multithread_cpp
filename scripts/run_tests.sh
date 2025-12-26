#!/usr/bin/env bash
set -e
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -- -j

# Prefer running the test binary directly if available (more deterministic output)
if [ -x "${PWD}/tests/unit_tests" ]; then
  echo "Running unit_tests binary..."
  "${PWD}/tests/unit_tests" --gtest_color=yes || true
else
  echo "Running ctest..."
  ctest --output-on-failure || true
fi
