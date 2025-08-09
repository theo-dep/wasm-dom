#pragma once

#include <emscripten/val.h>

#include <string>
#include <vector>

namespace wasmdom
{

    class DomRecycler
    {
    public:
        DomRecycler(bool useWasmGC);
        ~DomRecycler();

        emscripten::val create(const std::string& name);
        emscripten::val createNS(const std::string& name, const std::string& ns);
        emscripten::val createText(const std::string& text);
        emscripten::val createComment(const std::string& comment);

        void collect(emscripten::val node);

        // valid if no garbage collector
        std::vector<emscripten::val> nodes(const std::string& name) const;

    private:
        struct DomFactory;
        struct DomRecyclerFactory;
        DomFactory* _d_ptr;
    };

    DomRecycler& recycler();

}
