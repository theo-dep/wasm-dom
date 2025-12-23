#pragma once

#include "internals/domkeys.hpp"
#include "internals/jsapi.hpp"

#include <emscripten/val.h>

namespace wasmdom::internals
{
    class DomRecycler;

    struct DomFactory
    {
        static inline emscripten::val create(DomRecycler&, const std::string& name)
        {
            return emscripten::val::take_ownership(jsapi::createElement(name.c_str()));
        }
        static inline emscripten::val createNS(DomRecycler&, const std::string& name, const std::string& ns)
        {
            emscripten::val node = emscripten::val::take_ownership(jsapi::createElementNS(ns.c_str(), name.c_str()));
            node.set(nodeNSKey, ns);
            return node;
        }
        static inline emscripten::val createText(DomRecycler&, const std::string& text)
        {
            return emscripten::val::take_ownership(jsapi::createTextNode(text.c_str()));
        }
        static inline emscripten::val createComment(DomRecycler&, const std::string& comment)
        {
            return emscripten::val::take_ownership(jsapi::createComment(comment.c_str()));
        }

        static inline void collect(DomRecycler&, emscripten::val /*node*/) {} // // LCOV_EXCL_LINE
    };
}
