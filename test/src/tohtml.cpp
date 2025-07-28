#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

using namespace wasmdom;
using namespace wasmdom::dsl;

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
        VNode vnode = div();
        REQUIRE(vnode.toHTML() == "<div></div>");
    }

    SECTION("should parse comments")
    {
        VNode vnode = comment(std::string("comment"));
        REQUIRE(vnode.toHTML() == "<!--comment-->");
    }

    SECTION("should parse fragments")
    {
        VNode vnode =
            fragment()(
                { span(),
                  b() });
        REQUIRE(vnode.toHTML() == "<span></span><b></b>");
    }

    SECTION("should parse text")
    {
        VNode vnode = t("a text 字à");
        REQUIRE(vnode.toHTML() == "a text 字à");
    }

    SECTION("should handle children")
    {
        VNode vnode =
            div()(
                { span(),
                  b() });
        REQUIRE(vnode.toHTML() == "<div><span></span><b></b></div>");
    }

    SECTION("should handle text content")
    {
        VNode vnode =
            p()(
                std::string("a text 字à"));
        REQUIRE(vnode.toHTML() == "<p>a text 字à</p>");
    }

    SECTION("should parse attributes")
    {
        VNode vnode = div(("data-foo", "bar 字à"s));
        REQUIRE(vnode.toHTML() == "<div data-foo=\"bar 字à\"></div>");
    }

    SECTION("should omit falsy attributes")
    {
        VNode vnode =
            div(("readonly", "false"s),
                ("style", "width: 250px; height: 250px;"s));
        REQUIRE(vnode.toHTML() == "<div style=\"width: 250px; height: 250px;\"></div>");
    }

    SECTION("should set truthy attributes to empty string")
    {
        VNode vnode = div(("readonly", "true"s));
        REQUIRE(vnode.toHTML() == "<div readonly=\"\"></div>");
    }

    SECTION("should parse props")
    {
        VNode vnode = div(("readonly", emscripten::val(true)));
        REQUIRE(vnode.toHTML() == "<div readonly=\"true\"></div>");
    }

    SECTION("should omit props")
    {
        VNode vnode =
            div(("attributes", emscripten::val("foo")),
                ("childElementCount", emscripten::val("foo")),
                ("children", emscripten::val("foo")),
                ("classList", emscripten::val("foo")),
                ("clientHeight", emscripten::val("foo")),
                ("clientLeft", emscripten::val("foo")),
                ("clientTop", emscripten::val("foo")),
                ("clientWidth", emscripten::val("foo")),
                ("currentStyle", emscripten::val("foo")),
                ("firstElementChild", emscripten::val("foo")),
                ("innerHTML", emscripten::val("foo")),
                ("lastElementChild", emscripten::val("foo")),
                ("nextElementSibling", emscripten::val("foo")),
                ("ongotpointercapture", emscripten::val("foo")),
                ("onlostpointercapture", emscripten::val("foo")),
                ("onwheel", emscripten::val("foo")),
                ("outerHTML", emscripten::val("foo")),
                ("previousElementSibling", emscripten::val("foo")),
                ("runtimeStyle", emscripten::val("foo")),
                ("scrollHeight", emscripten::val("foo")),
                ("scrollLeft", emscripten::val("foo")),
                ("scrollLeftMax", emscripten::val("foo")),
                ("scrollTop", emscripten::val("foo")),
                ("scrollTopMax", emscripten::val("foo")),
                ("scrollWidth", emscripten::val("foo")),
                ("tabStop", emscripten::val("foo")),
                ("tagName", emscripten::val("foo")));
        REQUIRE(vnode.toHTML() == "<div>foo</div>");
    }

    SECTION("should omit callbacks")
    {
        VNode vnode =
            div(("onclick", [](emscripten::val /*e*/) -> bool {
                return true;
            }));
        REQUIRE(vnode.toHTML() == "<div></div>");
    }

    SECTION("should handle innerHTML")
    {
        VNode vnode = div(("innerHTML", emscripten::val::u8string("<p>a text 字à</p>")));
        REQUIRE(vnode.toHTML() == "<div><p>a text 字à</p></div>");
    }

    SECTION("should handle svg container elements")
    {
        VNode vnode =
            svg()(
                { a(),
                  defs(),
                  glyph(),
                  g(),
                  marker(),
                  mask(),
                  missing_glyph(),
                  pattern(),
                  svg(),
                  hswitch(),
                  symbol(),
                  text(),
                  desc(),
                  metadata(),
                  title() });
        REQUIRE(vnode.toHTML() == "<svg><a></a><defs></defs><glyph></glyph><g></g><marker></marker><mask></mask><missing-glyph></missing-glyph><pattern></pattern><svg></svg><switch></switch><symbol></symbol><text></text><desc></desc><metadata></metadata><title></title></svg>");
    }

    SECTION("should handle svg non container elements")
    {
        VNode vnode =
            svg()(
                { rect() });
        REQUIRE(vnode.toHTML() == "<svg><rect /></svg>");
    }

    SECTION("should handle void elements")
    {
        VNode vnode =
            div()(
                { area(),
                  base(),
                  br(),
                  col(),
                  embed(),
                  hr(),
                  img(),
                  input(),
                  keygen(),
                  link(),
                  meta(),
                  param(),
                  source(),
                  track(),
                  wbr() });
        REQUIRE(vnode.toHTML() == "<div><area><base><br><col><embed><hr><img><input><keygen><link><meta><param><source><track><wbr></div>");
    }

    SECTION("should escape text")
    {
        VNode vnode = t("<>\"'&`text");
        REQUIRE(vnode.toHTML() == "&lt;&gt;&quot;&apos;&amp;&#96;text");
    }

    SECTION("should escape text content")
    {
        VNode vnode =
            p()(
                std::string("<>\"'&`text"));
        REQUIRE(vnode.toHTML() == "<p>&lt;&gt;&quot;&apos;&amp;&#96;text</p>");
    }

    SECTION("should escape attributes")
    {
        VNode vnode = div(("data-foo", "<>\"'&`text"s));
        REQUIRE(vnode.toHTML() == "<div data-foo=\"&lt;&gt;&quot;&apos;&amp;&#96;text\"></div>");
    }

    SECTION("should escape props")
    {
        VNode vnode = div(("data-foo", emscripten::val("<>\"'&`text")));
        REQUIRE(vnode.toHTML() == "<div data-foo=\"&lt;&gt;&quot;&apos;&amp;&#96;text\"></div>");
    }
}
