#include <catch2/catch_test_macros.hpp>

#include "wasm-dom/attribute.hpp"
#include "wasm-dom/domkeys.hpp"
#include "wasm-dom/domrecycler.hpp"
#include "wasm-dom/internals/diff.hpp"

#include "jsdom.hpp"

using namespace wasmdom;

TEST_CASE("domRecycler", "[domRecycler]")
{
    const JSDom jsDom;

    wasmdom::DomRecycler recycler(false);

    SECTION("should access an unknown node")
    {
        REQUIRE(recycler.nodes("???").empty());
    }

    SECTION("should create and recycler texts")
    {
        emscripten::val node = recycler.createText("Hello World!");
        REQUIRE(node["nodeValue"].as<std::string>() == "Hello World!");

        REQUIRE(recycler.nodes("#TEXT").empty());

        recycler.collect(node);

        std::vector textList = recycler.nodes("#TEXT");
        REQUIRE(textList.size() == 1);
        REQUIRE(textList[0].strictlyEquals(node));

        emscripten::val newNode = recycler.createText("New Hello World!");
        REQUIRE(node["nodeValue"].as<std::string>() == "New Hello World!");
        REQUIRE(newNode.strictlyEquals(node));

        REQUIRE(recycler.nodes("#TEXT").empty());
    }

    SECTION("should create and recycler comments")
    {
        emscripten::val node = recycler.createComment("Hello World!");
        REQUIRE(node["nodeValue"].as<std::string>() == "Hello World!");
        REQUIRE(recycler.nodes("#COMMENT").empty());

        recycler.collect(node);

        std::vector commentList = recycler.nodes("#COMMENT");
        REQUIRE(commentList.size() == 1);
        REQUIRE(commentList[0].strictlyEquals(node));

        emscripten::val newNode = recycler.createComment("New Hello World!");
        REQUIRE(node["nodeValue"].as<std::string>() == "New Hello World!");
        REQUIRE(newNode.strictlyEquals(node));
        REQUIRE(recycler.nodes("#COMMENT").empty());
    }

    SECTION("should create and normal nodes")
    {
        emscripten::val node = recycler.create("div");
        REQUIRE(node["nodeName"].as<std::string>() == "DIV");
        REQUIRE(recycler.nodes("DIV").empty());

        recycler.collect(node);

        std::vector list = recycler.nodes("DIV");
        REQUIRE(list.size() == 1);
        REQUIRE(list[0].strictlyEquals(node));

        emscripten::val newNode = recycler.create("div");
        REQUIRE(node["nodeName"].as<std::string>() == "DIV");
        REQUIRE(newNode.strictlyEquals(node));
        REQUIRE(recycler.nodes("DIV").empty());
    }

    SECTION("should create nodes with namespace")
    {
        emscripten::val node = recycler.createNS("svg", "http://www.w3.org/2000/svg");
        REQUIRE(node["nodeName"].as<std::string>() == "svg");
        REQUIRE(recycler.nodes("SVGhttp://www.w3.org/2000/svg").empty());

        recycler.collect(node);

        std::vector list = recycler.nodes("SVGhttp://www.w3.org/2000/svg");
        REQUIRE(list.size() == 1);
        REQUIRE(list[0].strictlyEquals(node));

        emscripten::val newNode = recycler.createNS("svg", "http://www.w3.org/2000/svg");
        REQUIRE(node["nodeName"].as<std::string>() == "svg");
        REQUIRE(newNode.strictlyEquals(node));
        REQUIRE(recycler.nodes("SVGhttp://www.w3.org/2000/svg").empty());
    }

    SECTION("should clean children")
    {
        emscripten::val div = recycler.create("div");
        emscripten::val span = recycler.create("span");

        REQUIRE(div["children"]["length"].as<int>() == 0);

        div.call<void>("appendChild", span);

        REQUIRE(div["children"]["length"].as<int>() == 1);

        recycler.collect(div);

        REQUIRE(div["children"]["length"].as<int>() == 0);

        std::vector divList = recycler.nodes("DIV");
        std::vector spanList = recycler.nodes("SPAN");

        REQUIRE(divList.size() == 1);
        REQUIRE(divList[0].strictlyEquals(div));

        REQUIRE(spanList.size() == 1);
        REQUIRE(spanList[0].strictlyEquals(span));
    }

    SECTION("should clean attributes")
    {
        emscripten::val node = recycler.create("a");
        node.call<void>("setAttribute", std::string("href"), std::string("/foo"));
        REQUIRE(node.call<std::string>("getAttribute", std::string("href")) == "/foo");

        recycler.collect(node);

        emscripten::val attr = node.call<emscripten::val>("getAttribute", std::string("href"));
        REQUIRE((attr.isNull() || attr.isUndefined()));
    }

    SECTION("should clean props")
    {
        emscripten::val node = recycler.create("a");
        node.set("foo", emscripten::val("foo"));
        REQUIRE(node["foo"].as<std::string>() == "foo");

        recycler.collect(node);

        REQUIRE(node["foo"].isUndefined());
    }

    SECTION("should preserve wasmDom props")
    {
        emscripten::val node = recycler.create("a");
        node.set("wasmDomFoo", emscripten::val("foo"));
        REQUIRE(node["wasmDomFoo"].as<std::string>() == "foo");

        recycler.collect(node);

        REQUIRE(node["wasmDomFoo"].as<std::string>() == "foo");
    }

    SECTION("should clean textContent")
    {
        emscripten::val node = recycler.create("span");
        node.set("textContent", emscripten::val("foo"));
        REQUIRE(node["textContent"].as<std::string>() == "foo");

        recycler.collect(node);

        REQUIRE(node["textContent"].as<std::string>().empty());
    }

    SECTION("should clean wasmDomRaws")
    {
        emscripten::val node = recycler.create("span");

        // Create an empty function callback
        emscripten::val callback(Callback{});

        node.set("onclick", callback);
        node.set("onkeydown", callback);
        node.set(nodeRawsKey, emscripten::val::array(std::vector{ emscripten::val("onclick"), emscripten::val("onkeydown") }));

        REQUIRE(node["onclick"].strictlyEquals(callback));
        REQUIRE(node["onkeydown"].strictlyEquals(callback));

        recycler.collect(node);

        REQUIRE((node["onclick"].isNull() || node["onclick"].isUndefined()));
        REQUIRE((node["onkeydown"].isNull() || node["onkeydown"].isUndefined()));
        REQUIRE((node[nodeRawsKey].isNull() || node[nodeRawsKey].isUndefined()));
    }

    SECTION("should clean wasmDomEvents")
    {
        // calls counter
        int calls = 0;

        // Create callbacks incrementing calls
        emscripten::val callback(Callback([&](emscripten::val) { ++calls; return true; }));
        emscripten::val callbacks = emscripten::val::object();
        callbacks.set("click", callback);
        callbacks.set("keydown", callback);

        emscripten::val node = recycler.create("div");

        // Add event listeners
        node.call<void>("addEventListener", std::string("click"), emscripten::val::module_property(nodeEventProxyKey));
        node.call<void>("addEventListener", std::string("keydown"), emscripten::val::module_property(nodeEventProxyKey));

        node.set(nodeEventsKey, callbacks);

        // Trigger click event and check calls == 1
        node.call<void>("click");
        REQUIRE(calls == 1);

        // Collect and check asmDomEvents cleared
        recycler.collect(node);
        REQUIRE(node[nodeEventsKey].isUndefined());

        // Trigger click event again, calls must remain 1
        node.call<void>("click");
        REQUIRE(calls == 1);
    }
}
