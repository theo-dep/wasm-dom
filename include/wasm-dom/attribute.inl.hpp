#include "wasm-dom/conf.h"

WASMDOM_INLINE
bool wasmdom::Event::operator==(const Event&) const = default;

WASMDOM_INLINE
std::size_t wasmdom::EventHash::operator()(const Event& e) const
{
    return std::hash<std::size_t>{}(e.e);
}
