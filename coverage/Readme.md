From [clang LLVM 21.1.0-rc2](https://github.com/llvm/llvm-project/blob/c696ecddeea74fd3289234ea7b82083ef90c7189/compiler-rt/lib/profile/GCDAProfiling.c)

```sh
cmake --preset coverage
cmake --build build/coverage --target tests
node test/runner.js build/coverage/test/tests.js
lcov -b . -d build/coverage/src/CMakeFiles/wasm-dom.dir/wasm-dom/ --exclude erased -c -o build/coverage/lcov.info --gcov-tool coverage/llvm-gcov --no-external --ignore-errors format
genhtml build/coverage/lcov.info -o build/coverage/output --ignore-errors category
```
