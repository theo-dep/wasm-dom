See https://github.com/emscripten-core/emscripten/issues/13046#issuecomment-2655372720
From [nihui](https://github.com/nihui)

```sh
cmake --preset coverage
cmake --build build/coverage
node test/runner.js build/coverage/test/tests.js
lcov -b . -d build/coverage/src/CMakeFiles/wasm-dom.dir/wasm-dom/ -c -o build/coverage/lcov.info --gcov-tool coverage/llvm-gcov --no-external --ignore-errors format --ignore-errors inconsistent
genhtml build/coverage/lcov.info -o build/coverage/output --ignore-errors inconsistent --ignore-errors category
```
