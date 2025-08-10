#!/bin/sh

cmake --preset coverage
cmake --build build/coverage/ --target tests
find . -name '*.gcda' -delete -print
node test/runner.js build/coverage/test/tests.js
lcov -b . -d build/coverage/src/CMakeFiles/wasm-dom.dir/wasm-dom/ --exclude erased -c -o build/coverage/lcov.info --gcov-tool coverage/llvm-gcov --no-external --ignore-errors format
genhtml build/coverage/lcov.info -o build/coverage/output --ignore-errors category
