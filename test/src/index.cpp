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
        Config config = Config();
        init(config);

        VNode* vnode = h("div");
        VNode* vnode1 = h("div");
        ScopedVNode vnode2{ h("div") };

        VNode obj = *vnode;

        patch(getRoot(), vnode);
        patch(vnode, vnode1);
        patch(vnode1, vnode2.get());

        REQUIRE(vnode->sel != obj.sel);

        vnode = h("div");
        obj = *vnode;

        toHTML(vnode);

        REQUIRE(vnode->sel != obj.sel);
    }

    SECTION("should automatically clear memory (by config)")
    {
        Config config = Config();
        config.clearMemory = true;
        init(config);

        VNode* vnode = h("div");
        VNode* vnode1 = h("div");
        ScopedVNode vnode2{ h("div") };

        VNode obj = *vnode;

        patch(getRoot(), vnode);
        patch(vnode, vnode1);
        patch(vnode1, vnode2.get());

        REQUIRE(vnode->sel != obj.sel);

        vnode = h("div");
        obj = *vnode;

        toHTML(vnode);

        REQUIRE(vnode->sel != obj.sel);
    }

    SECTION("should not automatically clear memory (by config)")
    {
        Config config = Config();
        config.clearMemory = false;
        init(config);

        ScopedVNode vnode{ h("div") };
        ScopedVNode vnode1{ h("div") };
        ScopedVNode vnode2{ h("div") };

        VNode obj = *vnode;

        patch(getRoot(), vnode.get());
        patch(vnode.get(), vnode1.get());
        patch(vnode1.get(), vnode2.get());

        REQUIRE(vnode->sel == obj.sel);

        vnode.reset(h("div"));
        obj = *vnode;

        toHTML(vnode.get());

        REQUIRE(vnode->sel == obj.sel);
    }

    SECTION("should use safe patch")
    {
        Config config = Config();
        init(config);

        ScopedVNode vnode{ h("div") };
        VNode* vnode1 = h("div");
        ScopedVNode vnode2{ h("div") };

        REQUIRE(patch(getRoot(), vnode.get()) == vnode.get());
        REQUIRE(patch(vnode1, vnode2.get()) == nullptr);
    }

    SECTION("should use safe patch (by config)")
    {
        Config config = Config();
        config.unsafePatch = false;
        init(config);

        ScopedVNode vnode{ h("div") };
        VNode* vnode1 = h("div");
        ScopedVNode vnode2{ h("div") };

        REQUIRE(patch(getRoot(), vnode.get()) == vnode.get());
        REQUIRE(patch(vnode1, vnode2.get()) == nullptr);
    }

    SECTION("should not use safe patch (by config)")
    {
        Config config = Config();
        config.unsafePatch = true;
        init(config);

        ScopedVNode vnode{ h("div") };
        VNode* vnode1 = h("div");
        ScopedVNode vnode2{ h("div") };

        REQUIRE(patch(getRoot(), vnode.get()) == vnode.get());
        REQUIRE(patch(vnode1, vnode2.get()) == vnode2.get());
    }
}
