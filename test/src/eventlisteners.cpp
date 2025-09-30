#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "jsdom.hpp"

using namespace wasmdom;
using namespace wasmdom::dsl;

TEST_CASE("eventListeners", "[eventListeners]")
{
    std::vector<emscripten::val> result;
    auto callback = [&result](emscripten::val event) -> bool {
        std::string tagName = event["target"]["tagName"].as<std::string>();
        REQUIRE((tagName == "DIV" || tagName == "A"));

        result.push_back(event);
        return true;
    };

    const JSDom jsDom;

    SECTION("should attach a click event handler to element")
    {
        VNode vnode =
            div(("onclick", callback))(
                { a()("Click my parent") }
            );

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val node = jsDom.bodyFirstChild();

        node.call<void>("click");

        REQUIRE(result.size() == 1);
    }

    SECTION("should attach a click event handler without prefix to element")
    {
        VNode vnode =
            div(("click", callback))(
                { a()("Click my parent") }
            );

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val node = jsDom.bodyFirstChild();

        node.call<void>("click");

        REQUIRE(result.size() == 1);
    }

    SECTION("should detach attached click event handler to element")
    {
        VNode vnode =
            div(("onclick", callback))(
                { a()("Click my parent") }
            );

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val node = jsDom.bodyFirstChild();

        node.call<void>("click");

        REQUIRE(result.size() == 1);

        VNode vnode2 =
            div()(
                { a()("Click my parent") }
            );

        vdom.patch(vnode2);

        node = jsDom.bodyFirstChild();

        node.call<void>("click");

        REQUIRE(result.size() == 1);
    }

    SECTION("should share handlers in parent and child nodes")
    {
        VNode vnode =
            div(("onclick", callback))(
                { a(("onclick", callback))("Click my parent") }
            );

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val node = jsDom.bodyFirstChild();

        node.call<void>("click");

        REQUIRE(result.size() == 1);

        node["firstChild"].call<void>("click");

        REQUIRE(result.size() == 3);
    }

    SECTION("should handle lambda with capture")
    {
        int count = 1;

        VNode vnode =
            div(("onclick", [&count](emscripten::val /*e*/) -> bool {
                ++count;
                return false;
            }));

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val node = jsDom.bodyFirstChild();

        node.call<void>("click");

        REQUIRE(count == 2);
    }

    SECTION("should update handlers")
    {
        int count = 1;

        VNode vnode =
            div(("onclick", [&count](emscripten::val /*e*/) -> bool {
                ++count;
                return false;
            }));

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val node = jsDom.bodyFirstChild();

        node.call<void>("click");

        REQUIRE(count == 2);

        VNode vnode2 =
            div(("onclick", [&count](emscripten::val /*e*/) -> bool {
                --count;
                return false;
            }));

        vdom.patch(vnode2);

        node.call<void>("click");

        REQUIRE(count == 1);
    }

    SECTION("should not update handlers")
    {
        int count = 1;

        VNode vnode =
            div(("onclick", [&count](emscripten::val /*e*/) -> bool {
                ++count;
                return false;
            }));

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val node = jsDom.bodyFirstChild();

        node.call<void>("click");

        REQUIRE(count == 2);

        VNode vnode2 =
            div(("onclick", [&count](emscripten::val /*e*/) -> bool {
                ++count;
                return false;
            }));

        vdom.patch(vnode2);

        node.call<void>("click");

        REQUIRE(count == 3);
    }

    SECTION("should update handlers after deletion")
    {
        int count = 1;

        VDom vdom(jsDom.root());

        {
            VNode vnode =
                div(("onclick", [&count](emscripten::val /*e*/) -> bool {
                    ++count;
                    return false;
                }));

            vdom.patch(vnode);
        } // vnode is deleted here

        emscripten::val node = jsDom.bodyFirstChild();

        node.call<void>("click");

        REQUIRE(count == 2);
    }
}
