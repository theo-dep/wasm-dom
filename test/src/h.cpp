#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "utils.hpp"

using namespace wasmdom;

TEST_CASE("h", "[h]")
{
    SECTION("should delete a vnode")
    {
        VNode* vnode{
            h("div", {
                         h("span"),
                         h("div", h("video")),
                     })
        };
        deleteVNode(vnode);
    }

    SECTION("should create a vnode with a proper tag")
    {
        ScopedVNode vnode{ h("div") };
    }

    SECTION("should create a vnode with children")
    {
        ScopedVNode vnode{ h("div", { h("span"), h("b") }) };
    }

    SECTION("should create a vnode with one child")
    {
        ScopedVNode vnode{ h("div", h("span")) };
    }

    SECTION("should create a vnode with attrs and one child")
    {
        ScopedVNode vnode{
            h("div",
              { { "foo", "bar" } },
              h("span"))
        };
    }

    SECTION("should create a vnode with text content in string")
    {
        ScopedVNode vnode{ h("div", "I am a string") };
    }

    SECTION("should create a vnode for comment")
    {
        ScopedVNode vnode{ h("!", "test") };
    }

    SECTION("should create a vnode for fragment")
    {
        ScopedVNode vnode{ h("") };
    }

    SECTION("should create a vnode with attrs and text content in string")
    {
        ScopedVNode vnode{
            h("div",
              { { "foo", "bar" } },
              "I am a string")
        };
    }

    SECTION("should create a vnode with attrs and children")
    {
        ScopedVNode vnode{
            h("div",
              { { "foo", "bar" } },
              { h("span"), h("i") })
        };
    }

    SECTION("should create a vnode with text")
    {
        ScopedVNode vnode{ h("this is a text", true) };
    }

    SECTION("should create a vnode with attrs")
    {
        ScopedVNode vnode{
            h("i",
              { { "data-empty", "" },
                { "data-dash", "-" },
                { "data-dashed", "foo-bar" },
                { "data-camel", "fooBar" },
                { "data-integer", "0" },
                { "data-float", "0.1" } })
        };
    }

    SECTION("should create a vnode with props")
    {
        ScopedVNode vnode{
            h("i",
              { { "data-empty", emscripten::val("") },
                { "data-dash", emscripten::val("") },
                { "data-dashed", emscripten::val("foo-bar") },
                { "data-camel", emscripten::val("fooBar") },
                { "data-integer", emscripten::val(0) },
                { "data-float", emscripten::val(0.1) } })
        };
    }

    SECTION("should create a vnode with callbacks")
    {
        ScopedVNode vnode{
            h("i",
              {
                  { "onclick", onClick },
              })
        };
    }

    SECTION("should create a vnode with attrs and props")
    {
        ScopedVNode vnode{
            h("i",
              { { "data-empty", "" },
                { "data-dash", "-" },
                { "data-dashed", "foo-bar" },
                { "data-camel", "fooBar" },
                { "data-integer", "0" },
                { "data-float", "0.1" },
                { "data-empty-2", emscripten::val("") },
                { "data-dash-2", emscripten::val("") },
                { "data-dashed-2", emscripten::val("foo-bar") },
                { "data-camel-2", emscripten::val("fooBar") },
                { "data-integer-2", emscripten::val(0) },
                { "data-float-2", emscripten::val(0.1) } })
        };
    }

    SECTION("should create a vnode with attrs and callbacks")
    {
        ScopedVNode vnode{
            h("i",
              {
                  { "data-empty", "" },
                  { "data-dash", "-" },
                  { "data-dashed", "foo-bar" },
                  { "data-camel", "fooBar" },
                  { "data-integer", "0" },
                  { "data-float", "0.1" },
                  { "onclick", onClick },
              })
        };
    }

    SECTION("should create a vnode with props and callbacks")
    {
        ScopedVNode vnode{
            h("i",
              {
                  { "data-empty", emscripten::val("") },
                  { "data-dash", emscripten::val("") },
                  { "data-dashed", emscripten::val("foo-bar") },
                  { "data-camel", emscripten::val("fooBar") },
                  { "data-integer", emscripten::val(0) },
                  { "data-float", emscripten::val(0.1) },
                  { "onclick", onClick },
              })
        };
    }

    SECTION("should create a vnode with attrs, props and callbacks")
    {
        ScopedVNode vnode{
            h("i",
              {
                  { "data-empty", "" },
                  { "data-dash", "-" },
                  { "data-dashed", "foo-bar" },
                  { "data-camel", "fooBar" },
                  { "data-integer", "0" },
                  { "data-float", "0.1" },
                  { "data-empty-2", emscripten::val("") },
                  { "data-dash-2", emscripten::val("") },
                  { "data-dashed-2", emscripten::val("foo-bar") },
                  { "data-camel-2", emscripten::val("fooBar") },
                  { "data-integer-2", emscripten::val(0) },
                  { "data-float-2", emscripten::val(0.1) },
                  { "onclick", onClick },
              })
        };
    }
}
