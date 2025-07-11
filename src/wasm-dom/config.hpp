#pragma once

namespace wasmdom
{

    extern bool CLEAR_MEMORY;
    extern bool UNSAFE_PATCH;

    struct Config
    {
        bool clearMemory = true;
        bool unsafePatch = false;
    };

}
