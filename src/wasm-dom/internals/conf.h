#pragma once

#define WASMDOM_SH_INLINE

#ifdef WASMDOM_COVERAGE
#define WASMDOM_INLINE
#else
#define WASMDOM_INLINE inline
#endif

// for single header use, see https://github.com/emscripten-core/emscripten/issues/25219
// use WASMDOM_EM_JS instead of EM_JS in library mode for unicity
#define _WASMDOM_EM_JS(ret, c_name, js_name, params, code)                          \
    _EM_BEGIN_CDECL                                                                 \
    ret c_name params EM_IMPORT(js_name);                                           \
    __attribute__((weak, visibility("hidden"))) void* __em_js_ref_##c_name =        \
        (void*)&c_name;                                                             \
    EMSCRIPTEN_KEEPALIVE                                                            \
    __attribute__((weak, section("em_js"), aligned(1))) char __em_js__##js_name[] = \
        #params "<::>" code;                                                        \
    _EM_END_CDECL

#define WASMDOM_EM_JS(ret, name, params, ...) _WASMDOM_EM_JS(ret, name, name, params, #__VA_ARGS__)
