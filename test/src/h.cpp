#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "utils.hpp"

using namespace wasmdom;

TEST_CASE("h", "[h]")
{
    SECTION("should create a vnode with a proper tag")
    {
        VNode vnode{ h("div") };
    }

    SECTION("should create a vnode with children")
    {
        VNode vnode{ h("div", Children{ h("span"), h("b") }) };
    }

    SECTION("should create a vnode with one child")
    {
        VNode vnode{ h("div", h("span")) };
    }

    SECTION("should create a vnode with attrs and one child")
    {
        VNode vnode{
            h("div",
              Data(Attrs{
                  { "foo", "bar" } }),
              h("span"))
        };
    }

    SECTION("should create a vnode with text content in string")
    {
        VNode vnode{ h("div", "I am a string") };
    }

    SECTION("should create a vnode for comment")
    {
        VNode vnode{ h("!", "test") };
    }

    SECTION("should create a vnode for comment with attrs")
    {
        VNode vnode{ h("!", Data(Attrs{ { "foo", "bar" } }), "test") };
    }

    SECTION("should create a vnode for fragment")
    {
        VNode vnode{ h("") };
    }

    SECTION("should create a vnode with attrs and text content in string")
    {
        VNode vnode{
            h("div",
              Data(Attrs{
                  { "foo", "bar" } }),
              "I am a string")
        };
    }

    SECTION("should create a vnode with attrs and children")
    {
        VNode vnode{
            h("div",
              Data(Attrs{
                  { "foo", "bar" } }),
              Children{ h("span"), h("i") })
        };
    }

    SECTION("should create a vnode with text")
    {
        VNode vnode{ h("this is a text", true) };
    }

    SECTION("should create a vnode with random text")
    {
        VNode vnode{ h("fakeNode", false) };
    }

    SECTION("should create a vnode with attrs")
    {
        VNode vnode{
            h("i",
              Data(
                  Attrs{
                      { "data-empty", "" },
                      { "data-dash", "-" },
                      { "data-dashed", "foo-bar" },
                      { "data-camel", "fooBar" },
                      { "data-integer", "0" },
                      { "data-float", "0.1" } }))
        };
    }

    SECTION("should create a vnode with props")
    {
        VNode vnode{
            h("i",
              Data(
                  Props{
                      { "data-empty", emscripten::val("") },
                      { "data-dash", emscripten::val("") },
                      { "data-dashed", emscripten::val("foo-bar") },
                      { "data-camel", emscripten::val("fooBar") },
                      { "data-integer", emscripten::val(0) },
                      { "data-float", emscripten::val(0.1) } }))
        };
    }

    SECTION("should create a vnode with callbacks")
    {
        VNode vnode{
            h("i",
              Data(
                  Callbacks{
                      { "onclick", onClick },
                  }))
        };
    }

    SECTION("should create a vnode with attrs and props")
    {
        VNode vnode{
            h("i",
              Data(
                  Attrs{
                      { "data-empty", "" },
                      { "data-dash", "-" },
                      { "data-dashed", "foo-bar" },
                      { "data-camel", "fooBar" },
                      { "data-integer", "0" },
                      { "data-float", "0.1" } },
                  Props{
                      { "data-empty", emscripten::val("") },
                      { "data-dash", emscripten::val("") },
                      { "data-dashed", emscripten::val("foo-bar") },
                      { "data-camel", emscripten::val("fooBar") },
                      { "data-integer", emscripten::val(0) },
                      { "data-float", emscripten::val(0.1) } }))
        };
    }

    SECTION("should create a vnode with attrs and callbacks")
    {
        VNode vnode{
            h("i",
              Data(
                  Attrs{
                      { "data-empty", "" },
                      { "data-dash", "-" },
                      { "data-dashed", "foo-bar" },
                      { "data-camel", "fooBar" },
                      { "data-integer", "0" },
                      { "data-float", "0.1" } },
                  Callbacks{
                      { "onclick", onClick },
                  }))
        };
    }

    SECTION("should create a vnode with props and callbacks")
    {
        VNode vnode{
            h("i",
              Data(
                  Props{
                      { "data-empty", emscripten::val("") },
                      { "data-dash", emscripten::val("") },
                      { "data-dashed", emscripten::val("foo-bar") },
                      { "data-camel", emscripten::val("fooBar") },
                      { "data-integer", emscripten::val(0) },
                      { "data-float", emscripten::val(0.1) } },
                  Callbacks{
                      { "onclick", onClick },
                  }))
        };
    }

    SECTION("should create a vnode with attrs, props and callbacks")
    {
        VNode vnode{
            h("i",
              Data(
                  Attrs{
                      { "data-empty", "" },
                      { "data-dash", "-" },
                      { "data-dashed", "foo-bar" },
                      { "data-camel", "fooBar" },
                      { "data-integer", "0" },
                      { "data-float", "0.1" } },
                  Props{
                      { "data-empty", emscripten::val("") },
                      { "data-dash", emscripten::val("") },
                      { "data-dashed", emscripten::val("foo-bar") },
                      { "data-camel", emscripten::val("fooBar") },
                      { "data-integer", emscripten::val(0) },
                      { "data-float", emscripten::val(0.1) } },
                  Callbacks{
                      { "onclick", onClick },
                  }))
        };
    }
}
