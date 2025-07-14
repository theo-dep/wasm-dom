#pragma once

namespace wasmdom
{

    struct VNode;

    void diff(const VNode* __restrict__ const oldVnode, const VNode* __restrict__ const vnode);

}
