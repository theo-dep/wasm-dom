#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

using namespace wasmdom;
using namespace wasmdom::dsl;

inline bool onClick(emscripten::val /*event*/)
{
    return true;
}

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

WASMDOM_DSL_FOR_EACH(SEL_TEST_CASE, WASMDOM_DSL_ELEMENTS)
WASMDOM_DSL_FOR_EACH(SEL_TEST_CASE, WASMDOM_DSL_CONFLICT_ELEMENTS)
