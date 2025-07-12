#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

using namespace wasmdom;

TEST_CASE("should handle NULL VNode", "[toHTML]")
{
    VNode* vnode = nullptr;
    REQUIRE(toHTML(vnode) == "");
}

TEST_CASE("should parse elements")
{
    VNode* vnode = h("div");
    REQUIRE(toHTML(vnode) == "<div></div>");
}

TEST_CASE("should parse comments", "[toHTML]")
{
    VNode* vnode = h("!", std::string("comment"));
    REQUIRE(toHTML(vnode) == "<!--comment-->");
}

TEST_CASE("should parse fragments", "[toHTML]")
{
    VNode* vnode = h("", Children{ h("span"), h("b") });
    REQUIRE(toHTML(vnode) == "<span></span><b></b>");
}

TEST_CASE("should parse text", "[toHTML]")
{
    VNode* vnode = h("a text 字à", true);
    REQUIRE(toHTML(vnode) == "a text 字à");
}

TEST_CASE("should handle children", "[toHTML]")
{
    VNode* vnode = h("div", Children{ h("span"), h("b") });
    REQUIRE(toHTML(vnode) == "<div><span></span><b></b></div>");
}

TEST_CASE("should handle text content", "[toHTML]")
{
    VNode* vnode = h("p", std::string("a text 字à"));
    REQUIRE(toHTML(vnode) == "<p>a text 字à</p>");
}

TEST_CASE("should parse attributes", "[toHTML]")
{
    VNode* vnode = h("div", Attrs{ { "data-foo", "bar 字à" } });
    REQUIRE(toHTML(vnode) == "<div data-foo=\"bar 字à\"></div>");
}

TEST_CASE("should omit falsy attributes", "[toHTML]")
{
    VNode* vnode = h("div", Attrs{ { "readonly", "false" }, { "style", "width: 250px; height: 250px;" } });
    REQUIRE(toHTML(vnode) == "<div style=\"width: 250px; height: 250px;\"></div>");
}

TEST_CASE("should set truthy attributes to empty string", "[toHTML]")
{
    VNode* vnode = h("div", Attrs{ { "readonly", "true" } });
    REQUIRE(toHTML(vnode) == "<div readonly=\"\"></div>");
}

TEST_CASE("should parse props", "[toHTML]")
{
    VNode* vnode = h("div", Props{ { "readonly", emscripten::val(true) } });
    REQUIRE(toHTML(vnode) == "<div readonly=\"true\"></div>");
}

TEST_CASE("should omit props", "[toHTML]")
{
    VNode* vnode = h("div",
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
    REQUIRE(toHTML(vnode) == "<div>foo</div>");
}

TEST_CASE("should omit callbacks", "[toHTML]")
{
    VNode* vnode = h("div", Data(Callbacks{
                                { "onclick", [](emscripten::val e) -> bool {
                                     return true;
                                 } } }));
    REQUIRE(toHTML(vnode) == "<div></div>");
}

TEST_CASE("should handle innerHTML", "[toHTML]")
{
    VNode* vnode = h("div", Props{ { "innerHTML", emscripten::val::u8string("<p>a text 字à</p>") } });
    REQUIRE(toHTML(vnode) == "<div><p>a text 字à</p></div>");
}

TEST_CASE("should handle svg container elements", "[toHTML]")
{
    VNode* vnode = h("svg",
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
    REQUIRE(toHTML(vnode) == "<svg><a></a><defs></defs><glyph></glyph><g></g><marker></marker><mask></mask><missing-glyph></missing-glyph><pattern></pattern><svg></svg><switch></switch><symbol></symbol><text></text><desc></desc><metadata></metadata><title></title></svg>");
}

TEST_CASE("should handle svg non container elements", "[toHTML]")
{
    VNode* vnode = h("svg", Children{ h("rect") });
    REQUIRE(toHTML(vnode) == "<svg><rect /></svg>");
}

TEST_CASE("should handle void elements", "[toHTML]")
{
    VNode* vnode = h("div",
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
    REQUIRE(toHTML(vnode) == "<div><area><base><br><col><embed><hr><img><input><keygen><link><meta><param><source><track><wbr></div>");
}

TEST_CASE("should escape text", "[toHTML]")
{
    VNode* vnode = h("<>\"'&`text", true);
    REQUIRE(toHTML(vnode) == "&lt;&gt;&quot;&apos;&amp;&#96;text");
}

TEST_CASE("should escape text content", "[toHTML]")
{
    VNode* vnode = h("p", std::string("<>\"'&`text"));
    REQUIRE(toHTML(vnode) == "<p>&lt;&gt;&quot;&apos;&amp;&#96;text</p>");
}

TEST_CASE("should escape attributes", "[toHTML]")
{
    VNode* vnode = h("div", Attrs{ { "data-foo", "<>\"'&`text" } });
    REQUIRE(toHTML(vnode) == "<div data-foo=\"&lt;&gt;&quot;&apos;&amp;&#96;text\"></div>");
}

TEST_CASE("should escape props", "[toHTML]")
{
    VNode* vnode = h("div", Props{ { "data-foo", emscripten::val("<>\"'&`text") } });
    REQUIRE(toHTML(vnode) == "<div data-foo=\"&lt;&gt;&quot;&apos;&amp;&#96;text\"></div>");
}
