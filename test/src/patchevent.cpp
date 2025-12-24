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

    int refCount = 0;

    auto checkNode = [&](emscripten::val node) {
        REQUIRE(isHTMLElement(node.as_handle()));
        ++refCount;
    };

    SECTION("should call onMount when a node is added in DOM")
    {
        VNode vnode1 =
            div((onMount, checkNode))( // update
                { span(("key", "1"s), (onMount, checkNode)),
                  span(("key", "2"s), (onMount, checkNode)),
                  span(("key", "3"s), (onMount, checkNode)),
                  span(("key", "4"s), (onMount, checkNode)),
                  span(("key", "5"s), (onMount, checkNode)),
                  span(("key", "6"s), (onMount, checkNode)) }
            );

        VDom vdom(jsDom.root());
        vdom.patch(vnode1);

        REQUIRE(refCount == 6);

        VNode vnode2 =
            div((onMount, checkNode))(                                   // update
                { div(("id", "0"s), (onMount, checkNode))(               // new selector
                      span(("key", "10"s), (onMount, checkNode))(        // with child
                          { span(("key", "101"s), (onMount, checkNode)), // and grandchildren
                            span(("key", "102"s), (onMount, checkNode)) }
                      )
                  ),
                  span(("key", "6"s), (onMount, checkNode)),  // inverse from last
                  span(("key", "22"s), (onMount, checkNode)), // same but different
                  i(("key", "2"s), (onMount, checkNode)),     // same key but different selector
                  span(("key", "4"s), (onMount, checkNode)),  // inverse from middle
                  span(("key", "3"s), (onMount, checkNode)),  // inverse from middle
                  span(("key", "55"s), (onMount, checkNode)), // same but different
                  span(("key", "1"s), (onMount, checkNode)) } // inverse from first
            );

        vdom.patch(vnode2);

        REQUIRE(refCount == 8 + 9);

        VNode vnode3 = span((onMount, checkNode)); // change the root node

        vdom.patch(vnode3);

        REQUIRE(refCount == 8 + 9 + 1);
    }

    SECTION("should call onUpdate when a node is updated in DOM")
    {
        VNode vnode1 =
            div((onUpdate, checkNode))(
                { span(("key", "1"s), (onUpdate, checkNode)),  // add
                  span(("key", "2"s), (onUpdate, checkNode)) } // add
            );

        VDom vdom(jsDom.root());
        vdom.patch(vnode1);

        REQUIRE(refCount == 1);

        VNode vnode2 =
            div((onUpdate, checkNode))(
                { span(("key", "1"s), (onUpdate, checkNode)),  // update first
                  span(("key", "12"s), (onUpdate, checkNode)), // add
                  span(("key", "2"s), (onUpdate, checkNode)) } // update last
            );

        vdom.patch(vnode2);

        REQUIRE(refCount == 1 + 3);
    }

    SECTION("should call onUnmount when a node is removed from DOM")
    {
        VNode vnode1 =
            div((onUnmount, checkNode))(
                { div(("key", "1"s), (onUnmount, checkNode))(
                      span(("key", "10"s), (onUnmount, checkNode))
                  ),
                  span(("key", "2"s), (onUnmount, checkNode)),
                  span(("key", "3"s), (onUnmount, checkNode))(
                      { span(("key", "31"s), (onUnmount, checkNode)),
                        span(("key", "32"s), (onUnmount, checkNode))(
                            { span(("key", "321"s), (onUnmount, checkNode)),
                              span(("key", "322"s), (onUnmount, checkNode)) }
                        ),
                        span(("key", "33"s), (onUnmount, checkNode)) }
                  ),
                  span(("key", "4"s), (onUnmount, checkNode)),
                  span(("key", "5"s), (onUnmount, checkNode)),
                  span(("key", "6"s), (onUnmount, checkNode)) }
            );

        VDom vdom(jsDom.root());
        vdom.patch(vnode1);

        VNode vnode2 = div((onUnmount, checkNode)); // update

        vdom.patch(vnode2);

        REQUIRE(refCount == 12);

        VNode vnode3 = span((onUnmount, checkNode)); // change the root node

        vdom.patch(vnode3);

        REQUIRE(refCount == 12 + 1);
    }
}
