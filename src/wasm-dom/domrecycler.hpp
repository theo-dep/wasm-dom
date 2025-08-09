#pragma once

#include <emscripten/val.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace wasmdom
{

    class DomRecycler
    {
        DomRecycler() = default;
        friend DomRecycler& recycler();

    public:
        emscripten::val create(const std::string& name);
        emscripten::val createNS(const std::string& name, const std::string& ns);
        emscripten::val createText(const std::string& text);
        emscripten::val createComment(const std::string& comment);

        void collect(emscripten::val node);

        std::vector<emscripten::val> nodes(const std::string& name) const;

    private:
        std::unordered_map<std::string, std::vector<emscripten::val>> _nodes;
    };

    DomRecycler& recycler();

}
