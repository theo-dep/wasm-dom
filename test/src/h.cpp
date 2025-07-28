#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "utils.hpp"

using namespace wasmdom;
using namespace wasmdom::dsl;

TEST_CASE("h", "[h]")
{
    SECTION("should create a vnode for comment")
    {
        VNode vnode = comment("test");
        vnode = comment()("test");
    }

    SECTION("should create a vnode for fragment")
    {
        VNode vnode = fragment();
    }

    SECTION("should create a vnode with text")
    {
        VNode vnode = t("this is a text");
    }
}

#define SEL_TEST_CASE(X)                                                       \
    TEST_CASE(#X, "[h][" #X "]")                                               \
    {                                                                          \
        SECTION("should create a vnode with a proper tag")                     \
        {                                                                      \
            VNode vnode = X();                                                 \
        }                                                                      \
        SECTION("should create a vnode with children")                         \
        {                                                                      \
            VNode vnode = X()({ X(), X() });                                   \
        }                                                                      \
        SECTION("should create a vnode with one child")                        \
        {                                                                      \
            VNode vnode = X()(X());                                            \
        }                                                                      \
        SECTION("should create a vnode with attrs and one child")              \
        {                                                                      \
            VNode vnode = X(("foo", "bar"s))(X());                             \
        }                                                                      \
        SECTION("should create a vnode with text content in string")           \
        {                                                                      \
            VNode vnode = X()("I am a string");                                \
        }                                                                      \
        SECTION("should create a vnode with attrs and text content in string") \
        {                                                                      \
            VNode vnode = X(("foo", "bar"s))("I am a string");                 \
        }                                                                      \
        SECTION("should create a vnode with attrs and children")               \
        {                                                                      \
            VNode vnode = X(("foo", "bar"s))({ X(), X() });                    \
        }                                                                      \
        SECTION("should create a vnode with attrs")                            \
        {                                                                      \
            VNode vnode =                                                      \
                X(("data-empty", ""s),                                         \
                  ("data-dash", "-"s),                                         \
                  ("data-dashed", "foo-bar"s),                                 \
                  ("data-camel", "fooBar"s),                                   \
                  ("data-integer", "0"s),                                      \
                  ("data-float", "0.1"s));                                     \
        }                                                                      \
        SECTION("should create a vnode with props")                            \
        {                                                                      \
            VNode vnode =                                                      \
                X(("data-empty", emscripten::val("")),                         \
                  ("data-dash", emscripten::val("")),                          \
                  ("data-dashed", emscripten::val("foo-bar")),                 \
                  ("data-camel", emscripten::val("fooBar")),                   \
                  ("data-integer", emscripten::val(0)),                        \
                  ("data-float", emscripten::val(0.1)));                       \
        }                                                                      \
        SECTION("should create a vnode with callbacks")                        \
        {                                                                      \
            VNode vnode = X(("onclick", f(onClick)));                          \
        }                                                                      \
        SECTION("should create a vnode with attrs and props")                  \
        {                                                                      \
            VNode vnode =                                                      \
                X(("data-empty", ""s),                                         \
                  ("data-dash", "-"s),                                         \
                  ("data-dashed", "foo-bar"s),                                 \
                  ("data-camel", "fooBar"s),                                   \
                  ("data-integer", "0"s),                                      \
                  ("data-float", "0.1"s),                                      \
                  ("data-empty", emscripten::val("")),                         \
                  ("data-dash", emscripten::val("")),                          \
                  ("data-dashed", emscripten::val("foo-bar")),                 \
                  ("data-camel", emscripten::val("fooBar")),                   \
                  ("data-integer", emscripten::val(0)),                        \
                  ("data-float", emscripten::val(0.1)));                       \
        }                                                                      \
        SECTION("should create a vnode with attrs and callbacks")              \
        {                                                                      \
            VNode vnode =                                                      \
                X(("data-empty", ""s),                                         \
                  ("data-dash", "-"s),                                         \
                  ("data-dashed", "foo-bar"s),                                 \
                  ("data-camel", "fooBar"s),                                   \
                  ("data-integer", "0"s),                                      \
                  ("data-float", "0.1"s),                                      \
                  ("onclick", f(onClick)));                                    \
        }                                                                      \
        SECTION("should create a vnode with props and callbacks")              \
        {                                                                      \
            VNode vnode =                                                      \
                X(("data-empty", emscripten::val("")),                         \
                  ("data-dash", emscripten::val("")),                          \
                  ("data-dashed", emscripten::val("foo-bar")),                 \
                  ("data-camel", emscripten::val("fooBar")),                   \
                  ("data-integer", emscripten::val(0)),                        \
                  ("data-float", emscripten::val(0.1)),                        \
                  ("onclick", f(onClick)));                                    \
        }                                                                      \
        SECTION("should create a vnode with attrs, props and callbacks")       \
        {                                                                      \
            VNode vnode =                                                      \
                X(("data-empty", ""s),                                         \
                  ("data-dash", "-"s),                                         \
                  ("data-dashed", "foo-bar"s),                                 \
                  ("data-camel", "fooBar"s),                                   \
                  ("data-integer", "0"s),                                      \
                  ("data-float", "0.1"s),                                      \
                  ("data-empty", emscripten::val("")),                         \
                  ("data-dash", emscripten::val("")),                          \
                  ("data-dashed", emscripten::val("foo-bar")),                 \
                  ("data-camel", emscripten::val("fooBar")),                   \
                  ("data-integer", emscripten::val(0)),                        \
                  ("data-float", emscripten::val(0.1)),                        \
                  ("onclick", f(onClick)));                                    \
        }                                                                      \
    }

SEL_TEST_CASE(a)
SEL_TEST_CASE(b)
SEL_TEST_CASE(div)
SEL_TEST_CASE(h2)
SEL_TEST_CASE(i)
SEL_TEST_CASE(input)
SEL_TEST_CASE(foreignObject)
SEL_TEST_CASE(p)
SEL_TEST_CASE(span)
SEL_TEST_CASE(style)
SEL_TEST_CASE(svg)
SEL_TEST_CASE(htemplate)
SEL_TEST_CASE(web_component)

// void elements
SEL_TEST_CASE(area)
SEL_TEST_CASE(base)
SEL_TEST_CASE(br)
SEL_TEST_CASE(col)
SEL_TEST_CASE(embed)
SEL_TEST_CASE(hr)
SEL_TEST_CASE(img)
SEL_TEST_CASE(keygen)
SEL_TEST_CASE(link)
SEL_TEST_CASE(meta)
SEL_TEST_CASE(param)
SEL_TEST_CASE(source)
SEL_TEST_CASE(track)
SEL_TEST_CASE(wbr)

// svg elements
SEL_TEST_CASE(defs)
SEL_TEST_CASE(glyph)
SEL_TEST_CASE(g)
SEL_TEST_CASE(marker)
SEL_TEST_CASE(mask)
SEL_TEST_CASE(missing_glyph)
SEL_TEST_CASE(pattern)
SEL_TEST_CASE(rect)
SEL_TEST_CASE(hswitch)
SEL_TEST_CASE(symbol)
SEL_TEST_CASE(text)
SEL_TEST_CASE(desc)
SEL_TEST_CASE(metadata)
SEL_TEST_CASE(title)
