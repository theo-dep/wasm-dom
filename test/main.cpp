#include <catch2/catch_session.hpp>

#include "wasm-dom.hpp"

int main(int argc, char** argv)
{
    wasmdom::init();
    return Catch::Session().run(argc, argv);
}
