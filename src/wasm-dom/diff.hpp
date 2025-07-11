#pragma once

namespace wasmdom
{

    struct VNode;

    void diff(VNode* __restrict__ const oldVnode, VNode* __restrict__ const vnode);

}
