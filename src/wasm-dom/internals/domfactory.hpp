#pragma once

#include <emscripten/val.h>

namespace wasmdom
{
    class DomRecycler;
}

namespace wasmdom::internals
{
    struct DomFactory
    {
        static inline emscripten::val create(DomRecycler&, const std::string& name)
        {
            return emscripten::val::global("document").call<emscripten::val>("createElement", name);
        }
        static inline emscripten::val createNS(DomRecycler&, const std::string& name, const std::string& ns)
        {
            emscripten::val node = emscripten::val::global("document").call<emscripten::val>("createElementNS", ns, name);
            node.set(nodeNSKey, ns);
            return node;
        }
        static inline emscripten::val createText(DomRecycler&, const std::string& text)
        {
            return emscripten::val::global("document").call<emscripten::val>("createTextNode", text);
        }
        static inline emscripten::val createComment(DomRecycler&, const std::string& comment)
        {
            return emscripten::val::global("document").call<emscripten::val>("createComment", comment);
        }

        static inline void collect(DomRecycler&, emscripten::val /*node*/) {} // // LCOV_EXCL_LINE
    };
}
