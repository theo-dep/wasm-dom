#pragma once

#define WASMDOM_SH_INLINE

#ifdef WASMDOM_COVERAGE
#define WASMDOM_INLINE
#else
#define WASMDOM_INLINE inline
#endif
