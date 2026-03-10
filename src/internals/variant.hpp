#pragma once

namespace wasmdom::internals
{
    template <class... Ts>
    struct overloaded : Ts...
    {
        using Ts::operator()...;
    };
}
