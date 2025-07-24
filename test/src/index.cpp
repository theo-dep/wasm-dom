#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "utils.hpp"

using namespace wasmdom;

TEST_CASE("index", "[index]")
{
    setupDom();

    SECTION("should automatically clear memory")
    {
        VNode* vnode = h("div");
        VNode* vnode1 = h("div");
        VNode* vnode2 = h("div");

        VNode obj = *vnode;

        VDom vdom(getRoot());
        vdom.patch(vnode);
        vdom.patch(vnode1);
        vdom.patch(vnode2);

        REQUIRE(vnode->sel() != obj.sel());
    }
}
