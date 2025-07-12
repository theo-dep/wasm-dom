#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "utils.hpp"

using namespace wasmdom;

TEST_CASE("eventListeners", "[eventListeners]")
{
    std::vector<emscripten::val> result = std::vector<emscripten::val>();
    auto callback = [&result](emscripten::val event) -> bool {
        std::string tagName = event["target"]["tagName"].as<std::string>();
        REQUIRE((tagName == "DIV" || tagName == "A"));

        result.push_back(event);
        return true;
    };

    setupDom();

    SECTION("should attach a click event handler to element")
    {
        ScopedVNode vnode{
            h("div",
              Data(
                  Callbacks{
                      { "onclick", callback } }),
              Children{
                  h(std::string("a"), std::string("Click my parent")) })
        };

        patch(getRoot(), vnode.get());

        emscripten::val elm = getBodyFirstChild();

        elm.call<void>("click");

        REQUIRE(result.size() == 1);
    }

    SECTION("should detach attached click event handler to element")
    {
        ScopedVNode vnode{
            h("div",
              Data(
                  Callbacks{
                      { "onclick", callback } }),
              Children{
                  h(std::string("a"), std::string("Click my parent")) })
        };

        patch(getRoot(), vnode.get());

        emscripten::val elm = getBodyFirstChild();

        elm.call<void>("click");

        REQUIRE(result.size() == 1);

        ScopedVNode vnode2{
            h("div",
              Children{
                  h(std::string("a"), std::string("Click my parent")) })
        };

        patch(vnode.release(), vnode2.get());

        elm = getBodyFirstChild();

        elm.call<void>("click");

        REQUIRE(result.size() == 1);
    }

    SECTION("should share handlers in parent and child nodes")
    {
        ScopedVNode vnode{
            h("div",
              Data(
                  Callbacks{
                      { "onclick", callback } }),
              Children{
                  h("a",
                    Data(
                        Callbacks{
                            { "onclick", callback } }),
                    std::string("Click my parent")) })
        };

        patch(getRoot(), vnode.get());

        emscripten::val elm = getBodyFirstChild();

        elm.call<void>("click");

        REQUIRE(result.size() == 1);

        elm["firstChild"].call<void>("click");

        REQUIRE(result.size() == 3);
    }

    SECTION("should handle lambda with capture")
    {
        int count = 1;

        ScopedVNode vnode{
            h("div",
              Data(
                  Callbacks{
                      { "onclick", [&count](emscripten::val e) -> bool {
                           ++count;
                           return false;
                       } } }))
        };

        patch(getRoot(), vnode.get());

        emscripten::val elm = getBodyFirstChild();

        elm.call<void>("click");

        REQUIRE(count == 2);
    }

    SECTION("should update handlers")
    {
        int count = 1;

        ScopedVNode vnode{
            h("div",
              Data(
                  Callbacks{
                      { "onclick", [&count](emscripten::val e) -> bool {
                           ++count;
                           return false;
                       } } }))
        };

        patch(getRoot(), vnode.get());

        emscripten::val elm = getBodyFirstChild();

        elm.call<void>("click");

        REQUIRE(count == 2);

        ScopedVNode vnode2{
            h("div",
              Data(
                  Callbacks{
                      { "onclick", [&count](emscripten::val e) -> bool {
                           --count;
                           return false;
                       } } }))
        };

        patch(vnode.release(), vnode2.get());

        elm.call<void>("click");

        REQUIRE(count == 1);
    }

    SECTION("should not update handlers")
    {
        int count = 1;

        ScopedVNode vnode{
            h("div",
              Data(
                  Callbacks{
                      { "onclick", [&count](emscripten::val e) -> bool {
                           ++count;
                           return false;
                       } } }))
        };

        patch(getRoot(), vnode.get());

        emscripten::val elm = getBodyFirstChild();

        elm.call<void>("click");

        REQUIRE(count == 2);

        ScopedVNode vnode2{
            h("div",
              Data(
                  Callbacks{
                      { "onclick", [&count](emscripten::val e) -> bool {
                           ++count;
                           return false;
                       } } }))
        };

        patch(vnode.release(), vnode2.get());

        elm.call<void>("click");

        REQUIRE(count == 3);
    }

    SECTION("should not attach ref event handler to element")
    {
        ScopedVNode vnode{
            h("div",
              Data(
                  Callbacks{
                      { "ref", [](emscripten::val e) -> bool {
                           return false;
                       } } }))
        };

        patch(getRoot(), vnode.get());

        emscripten::val elm = getBodyFirstChild();

        emscripten::val keys = emscripten::val::global("Object").call<emscripten::val>("keys", elm["asmDomEvents"]);

        REQUIRE(keys["length"].strictlyEquals(emscripten::val(0)));

        ScopedVNode vnode2{ h("div") };

        patch(vnode.release(), vnode2.get());

        keys = emscripten::val::global("Object").call<emscripten::val>("keys", elm["asmDomEvents"]);

        REQUIRE(keys["length"].strictlyEquals(emscripten::val(0)));
    }
}
