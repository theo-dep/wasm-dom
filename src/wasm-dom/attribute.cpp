#include "attribute.hpp"

#ifdef WASMDOM_COVERAGE
#include "attribute.inl.cpp"

wasmdom::VNodeAttributes::VNodeAttributes() = default;
wasmdom::VNodeAttributes::VNodeAttributes(const VNodeAttributes& other) = default;
wasmdom::VNodeAttributes::VNodeAttributes(VNodeAttributes&& other) = default;
wasmdom::VNodeAttributes& wasmdom::VNodeAttributes::operator=(const VNodeAttributes& other) = default;
wasmdom::VNodeAttributes& wasmdom::VNodeAttributes::operator=(VNodeAttributes&& other) = default;
wasmdom::VNodeAttributes::~VNodeAttributes() = default;
#endif
