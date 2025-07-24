#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "utils.hpp"

using namespace wasmdom;

TEST_CASE("index", "[index]")
{
    setupDom();

    reset();

    SECTION("should automatically clear memory")
    {
        Config config;
        init(config);

        VNode* vnode = h("div");
        VNode* vnode1 = h("div");
        ScopedVNode vnode2{ h("div") };

        VNode obj = *vnode;

        VDom vdom;
        vdom.patch(getRoot(), vnode);
        vdom.patch(vnode, vnode1);
        vdom.patch(vnode1, vnode2.get());

        REQUIRE(vnode->sel() != obj.sel());
    }

    SECTION("should automatically clear memory (by config)")
    {
        Config config;
        config.clearMemory = true;
        init(config);

        VNode* vnode = h("div");
        VNode* vnode1 = h("div");
        ScopedVNode vnode2{ h("div") };

        VNode obj = *vnode;

        VDom vdom;
        vdom.patch(getRoot(), vnode);
        vdom.patch(vnode, vnode1);
        vdom.patch(vnode1, vnode2.get());

        REQUIRE(vnode->sel() != obj.sel());
    }

    SECTION("should not automatically clear memory (by config)")
    {
        Config config;
        config.clearMemory = false;
        init(config);

        ScopedVNode vnode{ h("div") };
        ScopedVNode vnode1{ h("div") };
        ScopedVNode vnode2{ h("div") };

        VNode obj = *vnode;

        VDom vdom;
        vdom.patch(getRoot(), vnode.get());
        vdom.patch(vnode.get(), vnode1.get());
        vdom.patch(vnode1.get(), vnode2.get());

        REQUIRE(vnode->sel() == obj.sel());
    }

    SECTION("should use safe patch")
    {
        Config config;
        init(config);

        ScopedVNode vnode{ h("div") };
        VNode* vnode1 = h("div");
        ScopedVNode vnode2{ h("div") };

        VDom vdom;
        REQUIRE(vdom.patch(getRoot(), vnode.get()) == vnode.get());
        REQUIRE(vdom.patch(vnode1, vnode2.get()) == nullptr);
    }

    SECTION("should use safe patch (by config)")
    {
        Config config;
        config.unsafePatch = false;
        init(config);

        ScopedVNode vnode{ h("div") };
        VNode* vnode1 = h("div");
        ScopedVNode vnode2{ h("div") };

        VDom vdom;
        REQUIRE(vdom.patch(getRoot(), vnode.get()) == vnode.get());
        REQUIRE(vdom.patch(vnode1, vnode2.get()) == nullptr);
    }

    SECTION("should not use safe patch (by config)")
    {
        Config config;
        config.unsafePatch = true;
        init(config);

        ScopedVNode vnode{ h("div") };
        VNode* vnode1 = h("div");
        ScopedVNode vnode2{ h("div") };

        VDom vdom;
        REQUIRE(vdom.patch(getRoot(), vnode.get()) == vnode.get());
        REQUIRE(vdom.patch(vnode1, vnode2.get()) == vnode2.get());
    }
}
