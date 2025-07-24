#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "utils.hpp"

#include <emscripten.h>

TEST_CASE("domRecycler", "[domRecycler]")
{
    setupDom();

    EM_ASM({
        Module.recycler.nodes = {};
        globalThis.recycler = Module.recycler;
    });

    emscripten::val recycler = emscripten::val::global("recycler");

    SECTION("should create and recycler texts")
    {
        emscripten::val node = recycler.call<emscripten::val>("createText", std::string("Hello World!"));
        REQUIRE(node["nodeValue"].as<std::string>() == "Hello World!");

        emscripten::val nodes = recycler["nodes"];
        REQUIRE(nodes["#TEXT"].isUndefined());

        recycler.call<void>("collect", node);

        emscripten::val textArray = nodes["#TEXT"];
        REQUIRE(textArray["length"].as<int>() == 1);
        REQUIRE(textArray[0].as<emscripten::val>() == node);

        emscripten::val newNode = recycler.call<emscripten::val>("createText", std::string("New Hello World!"));
        REQUIRE(node["nodeValue"].as<std::string>() == "New Hello World!");
        REQUIRE(newNode.strictlyEquals(node));

        REQUIRE(nodes["#TEXT"]["length"].as<int>() == 0);
    }

    SECTION("should create and recycler comments")
    {
        emscripten::val node = recycler.call<emscripten::val>("createComment", std::string("Hello World!"));
        REQUIRE(node["nodeValue"].as<std::string>() == "Hello World!");
        REQUIRE(recycler["nodes"]["#COMMENT"].isUndefined());

        recycler.call<void>("collect", node);

        emscripten::val commentArray = recycler["nodes"]["#COMMENT"];
        REQUIRE(commentArray["length"].as<int>() == 1);
        REQUIRE(commentArray[0].strictlyEquals(node));

        emscripten::val newNode = recycler.call<emscripten::val>("createComment", std::string("New Hello World!"));
        REQUIRE(node["nodeValue"].as<std::string>() == "New Hello World!");
        REQUIRE(newNode.strictlyEquals(node));
        REQUIRE(recycler["nodes"]["#COMMENT"]["length"].as<int>() == 0);
    }

    SECTION("should create and normal nodes")
    {
        emscripten::val node = recycler.call<emscripten::val>("create", std::string("div"));
        REQUIRE(node["nodeName"].as<std::string>() == "DIV");
        REQUIRE(recycler["nodes"]["DIV"].isUndefined());

        recycler.call<void>("collect", node);

        emscripten::val array = recycler["nodes"]["DIV"];
        REQUIRE(array["length"].as<int>() == 1);
        REQUIRE(array[0].strictlyEquals(node));

        emscripten::val newNode = recycler.call<emscripten::val>("create", std::string("div"));
        REQUIRE(node["nodeName"].as<std::string>() == "DIV");
        REQUIRE(newNode.strictlyEquals(node));
        REQUIRE(recycler["nodes"]["DIV"]["length"].as<int>() == 0);
    }

    SECTION("should create nodes with namespace")
    {
        emscripten::val node = recycler.call<emscripten::val>("createNS", std::string("svg"), std::string("http://www.w3.org/2000/svg"));
        REQUIRE(node["nodeName"].as<std::string>() == "svg");
        REQUIRE(recycler["nodes"]["SVGhttp://www.w3.org/2000/svg"].isUndefined());

        recycler.call<void>("collect", node);

        emscripten::val array = recycler["nodes"]["SVGhttp://www.w3.org/2000/svg"];
        REQUIRE(array["length"].as<int>() == 1);
        REQUIRE(array[0].strictlyEquals(node));

        emscripten::val newNode = recycler.call<emscripten::val>("createNS", std::string("svg"), std::string("http://www.w3.org/2000/svg"));
        REQUIRE(node["nodeName"].as<std::string>() == "svg");
        REQUIRE(newNode.strictlyEquals(node));
        REQUIRE(recycler["nodes"]["SVGhttp://www.w3.org/2000/svg"]["length"].as<int>() == 0);
    }

    SECTION("should clean children")
    {
        emscripten::val div = recycler.call<emscripten::val>("create", std::string("div"));
        emscripten::val span = recycler.call<emscripten::val>("create", std::string("span"));

        REQUIRE(div["children"]["length"].as<int>() == 0);

        div.call<void>("appendChild", span);

        REQUIRE(div["children"]["length"].as<int>() == 1);

        recycler.call<void>("collect", div);

        REQUIRE(div["children"]["length"].as<int>() == 0);

        emscripten::val divArray = recycler["nodes"]["DIV"];
        emscripten::val spanArray = recycler["nodes"]["SPAN"];

        REQUIRE(divArray["length"].as<int>() == 1);
        REQUIRE(divArray[0].strictlyEquals(div));

        REQUIRE(spanArray["length"].as<int>() == 1);
        REQUIRE(spanArray[0].strictlyEquals(span));
    }

    SECTION("should clean attributes")
    {
        emscripten::val node = recycler.call<emscripten::val>("create", std::string("a"));
        node.call<void>("setAttribute", std::string("href"), std::string("/foo"));
        REQUIRE(node.call<std::string>("getAttribute", std::string("href")) == "/foo");

        recycler.call<void>("collect", node);

        emscripten::val attr = node.call<emscripten::val>("getAttribute", std::string("href"));
        REQUIRE((attr.isNull() || attr.isUndefined()));
    }

    SECTION("should clean props")
    {
        emscripten::val node = recycler.call<emscripten::val>("create", std::string("a"));
        node.set("foo", emscripten::val("foo"));
        REQUIRE(node["foo"].as<std::string>() == "foo");

        recycler.call<void>("collect", node);

        REQUIRE(node["foo"].isUndefined());
    }

    SECTION("should preserve asmDom props")
    {
        emscripten::val node = recycler.call<emscripten::val>("create", std::string("a"));
        node.set("asmDomFoo", emscripten::val("foo"));
        REQUIRE(node["asmDomFoo"].as<std::string>() == "foo");

        recycler.call<void>("collect", node);

        REQUIRE(node["asmDomFoo"].as<std::string>() == "foo");
    }

    SECTION("should clean textContent")
    {
        emscripten::val node = recycler.call<emscripten::val>("create", std::string("span"));
        node.set("textContent", emscripten::val("foo"));
        REQUIRE(node["textContent"].as<std::string>() == "foo");

        recycler.call<void>("collect", node);

        REQUIRE(node["textContent"].as<std::string>().empty());
    }

    SECTION("should clean asmDomRaws")
    {
        emscripten::val node = recycler.call<emscripten::val>("create", std::string("span"));

        // Create an empty JS function callback
        emscripten::val callback = emscripten::val::global("Function").new_();

        node.set("onclick", callback);
        node.set("onkeydown", callback);
        node.set("asmDomRaws", emscripten::val::array(std::vector{ emscripten::val("onclick"), emscripten::val("onkeydown") }));

        REQUIRE(node["onclick"].strictlyEquals(callback));
        REQUIRE(node["onkeydown"].strictlyEquals(callback));

        recycler.call<void>("collect", node);

        REQUIRE((node["onclick"].isNull() || node["onclick"].isUndefined()));
        REQUIRE((node["onkeydown"].isNull() || node["onkeydown"].isUndefined()));
        REQUIRE((node["asmDomRaws"].isNull() || node["asmDomRaws"].isUndefined()));
    }

    SECTION("should clean asmDomEvents")
    {
        // calls counter in JS
        emscripten::val global = emscripten::val::global();
        global.set("calls", 0);

        // Create callbacks incrementing calls
        emscripten::val callbacks = emscripten::val::object();
        callbacks.set("click", emscripten::val::global("Function").new_(std::string("calls++;")));
        callbacks.set("keydown", emscripten::val::global("Function").new_(std::string("calls++;")));

        emscripten::val node = recycler.call<emscripten::val>("create", std::string("div"));

        // Add event listeners
        node.call<void>("addEventListener", std::string("click"), callbacks["click"]);
        node.call<void>("addEventListener", std::string("keydown"), callbacks["keydown"]);

        node.set("asmDomEvents", callbacks);

        // Trigger click event and check calls == 1
        node.call<void>("click");
        REQUIRE(global["calls"].as<int>() == 1);

        // Collect and check asmDomEvents cleared
        recycler.call<void>("collect", node);
        REQUIRE(node["asmDomEvents"].isUndefined());

        // Trigger click event again, calls must remain 1
        node.call<void>("click");
        REQUIRE(global["calls"].as<int>() == 1);
    }
}
