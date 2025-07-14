#include <catch2/catch_session.hpp>

#include "wasm-dom.hpp"

int main(int argc, char** argv)
{
    wasmdom::Config config;
    config.unsafePatch = true;

    wasmdom::init(config);

    return Catch::Session().run(argc, argv);
}
