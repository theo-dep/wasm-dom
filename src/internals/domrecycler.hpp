#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace emscripten
{
    class val;
}

namespace wasmdom::internals
{
    struct DomFactoryVTable;
    struct DomFactory;
    struct DomRecyclerFactory;

    class DomRecycler
    {
    public:
        DomRecycler(bool useWasmGC);
#ifdef WASMDOM_COVERAGE
        ~DomRecycler();
#endif

        emscripten::val create(const std::string& name);
        emscripten::val createNS(const std::string& name, const std::string& ns);
        emscripten::val createText(const std::string& text);
        emscripten::val createComment(const std::string& comment);

        void collect(emscripten::val node);

        // valid if no garbage collector
        std::vector<emscripten::val> nodes(const std::string& name) const;

    private:
        friend struct internals::DomFactory;
        friend struct internals::DomRecyclerFactory;
        const internals::DomFactoryVTable* _factory;

        std::unordered_map<std::string, std::vector<emscripten::val>> _nodes;
    };

}
