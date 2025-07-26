#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

using namespace wasmdom;

TEST_CASE("toHTML", "[toHTML]")
{
    SECTION("should handle nullptr VNode")
    {
        VNode vnode(nullptr);
        vnode.normalize();
        vnode.diff(nullptr);
        REQUIRE(vnode.toHTML() == "");
    }

    SECTION("should parse elements")
    {
        VNode vnode = h("div");
        REQUIRE(vnode.toHTML() == "<div></div>");
    }

    SECTION("should parse comments")
    {
        VNode vnode = h("!", std::string("comment"));
        REQUIRE(vnode.toHTML() == "<!--comment-->");
    }

    SECTION("should parse fragments")
    {
        VNode vnode = h("", Children{ h("span"), h("b") });
        REQUIRE(vnode.toHTML() == "<span></span><b></b>");
    }

    SECTION("should parse text")
    {
        VNode vnode = h("a text 字à", true);
        REQUIRE(vnode.toHTML() == "a text 字à");
    }

    SECTION("should handle children")
    {
        VNode vnode = h("div", Children{ h("span"), h("b") });
        REQUIRE(vnode.toHTML() == "<div><span></span><b></b></div>");
    }

    SECTION("should handle text content")
    {
        VNode vnode = h("p", std::string("a text 字à"));
        REQUIRE(vnode.toHTML() == "<p>a text 字à</p>");
    }

    SECTION("should parse attributes")
    {
        VNode vnode = h("div", Attrs{ { "data-foo", "bar 字à" } });
        REQUIRE(vnode.toHTML() == "<div data-foo=\"bar 字à\"></div>");
    }

    SECTION("should omit falsy attributes")
    {
        VNode vnode = h("div", Attrs{ { "readonly", "false" }, { "style", "width: 250px; height: 250px;" } });
        REQUIRE(vnode.toHTML() == "<div style=\"width: 250px; height: 250px;\"></div>");
    }

    SECTION("should set truthy attributes to empty string")
    {
        VNode vnode = h("div", Attrs{ { "readonly", "true" } });
        REQUIRE(vnode.toHTML() == "<div readonly=\"\"></div>");
    }

    SECTION("should parse props")
    {
        VNode vnode = h("div", Props{ { "readonly", emscripten::val(true) } });
        REQUIRE(vnode.toHTML() == "<div readonly=\"true\"></div>");
    }

    SECTION("should omit props")
    {
        VNode vnode = h("div",
                        Props{
                            { "attributes", emscripten::val("foo") },
                            { "childElementCount", emscripten::val("foo") },
                            { "children", emscripten::val("foo") },
                            { "classList", emscripten::val("foo") },
                            { "clientHeight", emscripten::val("foo") },
                            { "clientLeft", emscripten::val("foo") },
                            { "clientTop", emscripten::val("foo") },
                            { "clientWidth", emscripten::val("foo") },
                            { "currentStyle", emscripten::val("foo") },
                            { "firstElementChild", emscripten::val("foo") },
                            { "innerHTML", emscripten::val("foo") },
                            { "lastElementChild", emscripten::val("foo") },
                            { "nextElementSibling", emscripten::val("foo") },
                            { "ongotpointercapture", emscripten::val("foo") },
                            { "onlostpointercapture", emscripten::val("foo") },
                            { "onwheel", emscripten::val("foo") },
                            { "outerHTML", emscripten::val("foo") },
                            { "previousElementSibling", emscripten::val("foo") },
                            { "runtimeStyle", emscripten::val("foo") },
                            { "scrollHeight", emscripten::val("foo") },
                            { "scrollLeft", emscripten::val("foo") },
                            { "scrollLeftMax", emscripten::val("foo") },
                            { "scrollTop", emscripten::val("foo") },
                            { "scrollTopMax", emscripten::val("foo") },
                            { "scrollWidth", emscripten::val("foo") },
                            { "tabStop", emscripten::val("foo") },
                            { "tagName", emscripten::val("foo") } });
        REQUIRE(vnode.toHTML() == "<div>foo</div>");
    }

    SECTION("should omit callbacks")
    {
        VNode vnode = h("div", Data(Callbacks{
                                   { "onclick", [](emscripten::val /*e*/) -> bool {
                                        return true;
                                    } } }));
        REQUIRE(vnode.toHTML() == "<div></div>");
    }

    SECTION("should handle innerHTML")
    {
        VNode vnode = h("div", Props{ { "innerHTML", emscripten::val::u8string("<p>a text 字à</p>") } });
        REQUIRE(vnode.toHTML() == "<div><p>a text 字à</p></div>");
    }

    SECTION("should handle svg container elements")
    {
        VNode vnode = h("svg",
                        Children{
                            h("a"),
                            h("defs"),
                            h("glyph"),
                            h("g"),
                            h("marker"),
                            h("mask"),
                            h("missing-glyph"),
                            h("pattern"),
                            h("svg"),
                            h("switch"),
                            h("symbol"),
                            h("text"),
                            h("desc"),
                            h("metadata"),
                            h("title") });
        REQUIRE(vnode.toHTML() == "<svg><a></a><defs></defs><glyph></glyph><g></g><marker></marker><mask></mask><missing-glyph></missing-glyph><pattern></pattern><svg></svg><switch></switch><symbol></symbol><text></text><desc></desc><metadata></metadata><title></title></svg>");
    }

    SECTION("should handle svg non container elements")
    {
        VNode vnode = h("svg", Children{ h("rect") });
        REQUIRE(vnode.toHTML() == "<svg><rect /></svg>");
    }

    SECTION("should handle void elements")
    {
        VNode vnode = h("div",
                        Children{
                            h("area"),
                            h("base"),
                            h("br"),
                            h("col"),
                            h("embed"),
                            h("hr"),
                            h("img"),
                            h("input"),
                            h("keygen"),
                            h("link"),
                            h("meta"),
                            h("param"),
                            h("source"),
                            h("track"),
                            h("wbr") });
        REQUIRE(vnode.toHTML() == "<div><area><base><br><col><embed><hr><img><input><keygen><link><meta><param><source><track><wbr></div>");
    }

    SECTION("should escape text")
    {
        VNode vnode = h("<>\"'&`text", true);
        REQUIRE(vnode.toHTML() == "&lt;&gt;&quot;&apos;&amp;&#96;text");
    }

    SECTION("should escape text content")
    {
        VNode vnode = h("p", std::string("<>\"'&`text"));
        REQUIRE(vnode.toHTML() == "<p>&lt;&gt;&quot;&apos;&amp;&#96;text</p>");
    }

    SECTION("should escape attributes")
    {
        VNode vnode = h("div", Attrs{ { "data-foo", "<>\"'&`text" } });
        REQUIRE(vnode.toHTML() == "<div data-foo=\"&lt;&gt;&quot;&apos;&amp;&#96;text\"></div>");
    }

    SECTION("should escape props")
    {
        VNode vnode = h("div", Props{ { "data-foo", emscripten::val("<>\"'&`text") } });
        REQUIRE(vnode.toHTML() == "<div data-foo=\"&lt;&gt;&quot;&apos;&amp;&#96;text\"></div>");
    }
}
