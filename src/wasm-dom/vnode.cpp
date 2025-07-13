#include "vnode.hpp"

#include "vnodeforward.hpp"

#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <cstdint>
#include <string>
#include <unordered_map>

namespace wasmdom
{

    unsigned int currentHash = 0;
    std::unordered_map<std::string, unsigned int> hashes;

}

void wasmdom::VNode::normalize(const bool injectSvgNamespace)
{
    if (!(hash & isNormalized)) {
        if (data.attrs.count("key")) {
            hash |= hasKey;
            key = data.attrs["key"];
            data.attrs.erase("key");
        }

        if (sel[0] == '!') {
            hash |= isComment;
            sel = "";
        } else {
            children.erase(std::remove(children.begin(), children.end(), (VNode*)nullptr), children.end());

            Attrs::iterator it = data.attrs.begin();
            while (it != data.attrs.end()) {
                if (it->first == "ns") {
                    hash |= hasNS;
                    ns = it->second;
                    it = data.attrs.erase(it);
                } else if (it->second == "false") {
                    it = data.attrs.erase(it);
                } else {
                    if (it->second == "true") {
                        it->second = "";
                    }
                    ++it;
                }
            }

            bool addNS = injectSvgNamespace || (sel[0] == 's' && sel[1] == 'v' && sel[2] == 'g');
            if (addNS) {
                hash |= hasNS;
                ns = "http://www.w3.org/2000/svg";
            }

            if (!data.attrs.empty())
                hash |= hasAttrs;
            if (!data.props.empty())
                hash |= hasProps;
            if (!data.callbacks.empty())
                hash |= hasCallbacks;
            if (!children.empty()) {
                hash |= hasDirectChildren;

                Children::size_type i = children.size();
                while (i--) {
                    children[i]->normalize(
                        addNS && sel != "foreignObject");
                }
            }

            if (sel[0] == '\0') {
                hash |= isFragment;
            } else {
                if (hashes[sel] == 0) {
                    hashes[sel] = ++currentHash;
                }

                hash |= (hashes[sel] << 13) | isElement;

                if ((hash & hasCallbacks) && data.callbacks.count("ref")) {
                    hash |= hasRef;
                }
            }
        }

        hash |= isNormalized;
    }
}

void wasmdom::deleteVNode(const VNode* const vnode)
{
    if (!(vnode->hash & hasText)) {
        Children::size_type i = vnode->children.size();
        while (i--)
            deleteVNode(vnode->children[i]);
    }
    delete vnode;
}

wasmdom::VNode::~VNode()
{
    if (hash & hasText) {
        Children::size_type i = children.size();
        while (i--)
            delete children[i];
    }
}

namespace wasmdom
{

    emscripten::val functionCallback(const std::uintptr_t& vnode, std::string callback, emscripten::val event)
    {
        Callbacks cbs = reinterpret_cast<VNode*>(vnode)->data.callbacks;
        if (!cbs.count(callback)) {
            callback = "on" + callback;
        }
        return emscripten::val(cbs[callback](event));
    }

}

EMSCRIPTEN_BINDINGS(function_callback)
{
    emscripten::function("functionCallback", &wasmdom::functionCallback, emscripten::allow_raw_pointers());
}
