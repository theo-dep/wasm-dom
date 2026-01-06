#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "jsdom.hpp"

#include <emscripten.h>

using namespace wasmdom;
using namespace wasmdom::dsl;

EM_JS(bool, isHTMLElement, (emscripten::EM_VAL handle), {
    const obj = Emval.toValue(handle);
    return typeof obj === "object" && typeof obj.nodeType === "number" && typeof obj.nodeName === "string";
});

TEST_CASE("patchEvent", "[patchEvent]")
{
    const JSDom jsDom;

    int refMountCount = 0;
    int refUnmountCount = 0;
    int refUpdateCount = 0;

#define ADD_EVENTS                                    \
    (onMount, [&](emscripten::val node) {             \
        REQUIRE(isHTMLElement(node.as_handle()));     \
        ++refMountCount;                              \
    }),                                               \
        (onUnmount, [&](emscripten::val node) {       \
            REQUIRE(isHTMLElement(node.as_handle())); \
            ++refUnmountCount;                        \
        }),                                           \
        (onUpdate, [&](emscripten::val node) {        \
            REQUIRE(isHTMLElement(node.as_handle())); \
            ++refUpdateCount;                         \
        })

    SECTION("should call onMount when a node is added in DOM")
    {
        VNode vnode1 =
            div(ADD_EVENTS)( // update
                { span(("key", "1"s), ADD_EVENTS),
                  span(("key", "2"s), ADD_EVENTS),
                  span(("key", "3"s), ADD_EVENTS),
                  span(("key", "4"s), ADD_EVENTS),
                  span(("key", "5"s), ADD_EVENTS),
                  span(("key", "6"s), ADD_EVENTS) }
            );

        VDom vdom(jsDom.root());
        vdom.patch(vnode1);

        REQUIRE(refMountCount == 6);
        REQUIRE(refUnmountCount == 0);
        REQUIRE(refUpdateCount == 1);

        VNode vnode2 =
            div(ADD_EVENTS)(                                   // update
                { div(("id", "0"s), ADD_EVENTS)(               // new selector
                      span(("key", "10"s), ADD_EVENTS)(        // with child
                          { span(("key", "101"s), ADD_EVENTS), // and grandchildren
                            span(("key", "102"s), ADD_EVENTS) }
                      )
                  ),
                  span(("key", "6"s), ADD_EVENTS),  // inverse from last
                  span(("key", "22"s), ADD_EVENTS), // same but different
                  i(("key", "2"s), ADD_EVENTS),     // same key but different selector
                  span(("key", "4"s), ADD_EVENTS),  // inverse from middle
                  span(("key", "3"s), ADD_EVENTS),  // inverse from middle
                  span(("key", "55"s), ADD_EVENTS), // same but different
                  span(("key", "1"s), ADD_EVENTS) } // inverse from first
            );

        vdom.patch(vnode2);

        REQUIRE(refMountCount == 6 + 9);
        REQUIRE(refUnmountCount == 0 + 2);
        REQUIRE(refUpdateCount == 1 + 5);

        VNode vnode3 = span(ADD_EVENTS); // change the root node

        vdom.patch(vnode3);

        REQUIRE(refMountCount == 6 + 9 + 1);
        REQUIRE(refUnmountCount == 0 + 2 + 12);
        REQUIRE(refUpdateCount == 1 + 5 + 0);
    }

    SECTION("should call onUpdate when a node is updated in DOM")
    {
        VNode vnode1 =
            div(ADD_EVENTS)(
                { span(("key", "1"s), ADD_EVENTS),  // add
                  span(("key", "2"s), ADD_EVENTS) } // add
            );

        VDom vdom(jsDom.root());
        vdom.patch(vnode1);

        REQUIRE(refMountCount == 2);
        REQUIRE(refUnmountCount == 0);
        REQUIRE(refUpdateCount == 1);

        VNode vnode2 =
            div(ADD_EVENTS)(
                { span(("key", "1"s), ADD_EVENTS),  // update first
                  span(("key", "12"s), ADD_EVENTS), // add
                  span(("key", "2"s), ADD_EVENTS) } // update last
            );

        vdom.patch(vnode2);

        REQUIRE(refMountCount == 2 + 1);
        REQUIRE(refUnmountCount == 0 + 0);
        REQUIRE(refUpdateCount == 1 + 3);
    }

    SECTION("should call onUnmount when a node is removed from DOM")
    {
        VNode vnode1 =
            div(ADD_EVENTS)(
                { div(("key", "1"s), ADD_EVENTS)(
                      span(("key", "10"s), ADD_EVENTS)
                  ),
                  span(("key", "2"s), ADD_EVENTS),
                  span(("key", "3"s), ADD_EVENTS)(
                      { span(("key", "31"s), ADD_EVENTS),
                        span(("key", "32"s), ADD_EVENTS)(
                            { span(("key", "321"s), ADD_EVENTS),
                              span(("key", "322"s), ADD_EVENTS) }
                        ),
                        span(("key", "33"s), ADD_EVENTS) }
                  ),
                  span(("key", "4"s), ADD_EVENTS),
                  span(("key", "5"s), ADD_EVENTS),
                  span(("key", "6"s), ADD_EVENTS) }
            );

        VDom vdom(jsDom.root());
        vdom.patch(vnode1);

        REQUIRE(refMountCount == 12);
        REQUIRE(refUnmountCount == 0);
        REQUIRE(refUpdateCount == 1);

        VNode vnode2 = div(ADD_EVENTS); // update

        vdom.patch(vnode2);

        REQUIRE(refMountCount == 12 + 0);
        REQUIRE(refUnmountCount == 0 + 12);
        REQUIRE(refUpdateCount == 1 + 1);

        VNode vnode3 = span(ADD_EVENTS); // change the root node

        vdom.patch(vnode3);

        REQUIRE(refMountCount == 12 + 0 + 1);
        REQUIRE(refUnmountCount == 0 + 12 + 1);
        REQUIRE(refUpdateCount == 1 + 1 + 0);
    }
}
