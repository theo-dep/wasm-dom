#pragma once

#include "config.hpp"

namespace wasmdom
{

    inline Config& config()
    {
        static Config config;
        return config;
    }

}
