#pragma once

#include <emscripten/val.h>
#include <erased/erased.h>

#include <string>
#include <vector>

namespace wasmdom
{

    // LCOV_EXCL_START
    ERASED_MAKE_BEHAVIOR(
        Create, create,
        (&self, const std::string& name) requires(self.create(name))->emscripten::val
    );
    ERASED_MAKE_BEHAVIOR(
        CreateNS, createNS,
        (&self, const std::string& name, const std::string& ns) requires(self.createNS(name, ns))->emscripten::val
    );
    ERASED_MAKE_BEHAVIOR(
        CreateText, createText,
        (&self, const std::string& text) requires(self.createText(text))->emscripten::val
    );
    ERASED_MAKE_BEHAVIOR(
        CreateComment, createComment,
        (&self, const std::string& comment) requires(self.createComment(comment))->emscripten::val
    );
    ERASED_MAKE_BEHAVIOR(
        Collect, collect,
        (&self, emscripten::val node) requires(self.collect(node))->void
    );
    // LCOV_EXCL_STOP
    using DomFactory = erased::erased<Create, CreateNS, CreateText, CreateComment, Collect>;

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
        DomFactory _factory;
    };

    DomRecycler& recycler();

}
