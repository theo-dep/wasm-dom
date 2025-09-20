#!/bin/sh

cmake --preset coverage
cmake --build build/coverage/ --target tests
find . -name '*.gcda' -delete -print
node build/coverage/test/src/tests.js
lcov -b . -d build/coverage/src/CMakeFiles/wasm-dom.dir/wasm-dom/ -c -o build/coverage/lcov.info --gcov-tool coverage/llvm-gcov --no-external --ignore-errors format
genhtml build/coverage/lcov.info -o build/coverage/output --ignore-errors category
