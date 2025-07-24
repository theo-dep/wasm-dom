#pragma once

namespace wasmdom
{

    struct Config
    {
        bool clearMemory = true;
        bool unsafePatch = false;
    };

    void init(const Config& config);

}
