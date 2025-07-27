#pragma once

#include "vnode.hpp"

#define PARENS ()

#define EXPAND(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) __VA_ARGS__

#define FOR_EACH(macro, ...) \
    __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER(macro, a1, ...) \
    macro a1 __VA_OPT__(, FOR_EACH_AGAIN PARENS(macro, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER

#define t(Text) wasmdom::VNode(wasmdom::text, Text)

#define SEL(X, ...) wasmdom::VNode(#X __VA_OPT__(, ) FOR_EACH(std::pair, __VA_ARGS__))

#define a(...) SEL(a __VA_OPT__(, ) __VA_ARGS__)
#define div(...) SEL(div __VA_OPT__(, ) __VA_ARGS__)
#define span(...) SEL(span __VA_OPT__(, ) __VA_ARGS__)
