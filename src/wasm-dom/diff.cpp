#include "diff.hpp"

#include "vnode.hpp"

#include <emscripten.h>
#include <emscripten/val.h>

#include <cstdint>
#include <iterator>
#include <map>

namespace wasmdom
{

    void diffAttrs(VNode* __restrict__ const oldVnode, VNode* __restrict__ const vnode)
    {
        Attrs& oldAttrs = oldVnode->data.attrs;
        Attrs& attrs = vnode->data.attrs;

        for (const auto& it : oldAttrs) {
            if (!attrs.count(it.first)) {
                EM_ASM_({ Module.removeAttribute(
                              $0,
                              Module['UTF8ToString']($1)); }, vnode->elm, it.first.c_str());
            }
        }

        for (const auto& it : attrs) {
            if (!oldAttrs.count(it.first) || oldAttrs[it.first] != it.second) {
                EM_ASM_({ Module.setAttribute(
                              $0,
                              Module['UTF8ToString']($1),
                              Module['UTF8ToString']($2)); }, vnode->elm, it.first.c_str(), it.second.c_str());
            }
        }
    }

    void diffProps(const VNode* __restrict__ const oldVnode, const VNode* __restrict__ const vnode)
    {
        const Props& oldProps = oldVnode->data.props;
        const Props& props = vnode->data.props;

        emscripten::val elm = emscripten::val::module_property("nodes")[vnode->elm];

        EM_ASM_({ Module['nodes'][$0]['asmDomRaws'] = []; }, vnode->elm);

        for (const auto& it : oldProps) {
            if (!props.count(it.first)) {
                elm.set(it.first.c_str(), emscripten::val::undefined());
            }
        }

        for (const auto& it : props) {
            EM_ASM_({ Module['nodes'][$0]['asmDomRaws'].push(Module['UTF8ToString']($1)); }, vnode->elm, it.first.c_str());

            if (
                !oldProps.count(it.first) ||
                !it.second.strictlyEquals(oldProps.at(it.first)) ||
                ((it.first == "value" || it.first == "checked") &&
                 !it.second.strictlyEquals(elm[it.first.c_str()]))) {
                elm.set(it.first.c_str(), it.second);
            }
        }
    }

    void diffCallbacks(const VNode* __restrict__ const oldVnode, const VNode* __restrict__ const vnode)
    {
        const Callbacks& oldCallbacks = oldVnode->data.callbacks;
        const Callbacks& callbacks = vnode->data.callbacks;

        for (const auto& it : oldCallbacks) {
            if (!callbacks.count(it.first) && it.first != "ref") {
                EM_ASM_({
					var key = Module['UTF8ToString']($1).replace(/^on/, "");
					var elm = Module['nodes'][$0];
					elm.removeEventListener(
						key,
						Module['eventProxy'],
						false
					);
					delete elm['asmDomEvents'][key]; }, vnode->elm, it.first.c_str());
            }
        }

        EM_ASM_({
			var elm = Module['nodes'][$0];
			elm['asmDomVNode'] = $1;
			if (elm['asmDomEvents'] === undefined) {
				elm['asmDomEvents'] = {};
			} }, vnode->elm, reinterpret_cast<std::uintptr_t>(vnode));

        for (const auto& it : callbacks) {
            if (!oldCallbacks.count(it.first) && it.first != "ref") {
                EM_ASM_({
					var key = Module['UTF8ToString']($1).replace(/^on/, "");
					var elm = Module['nodes'][$0];
					elm.addEventListener(
						key,
						Module['eventProxy'],
						false
					);
					elm['asmDomEvents'][key] = Module['eventProxy']; }, vnode->elm, it.first.c_str());
            }
        }

        if (vnode->hash & hasRef) {
            bool (*const* callback)(emscripten::val) = callbacks.at("ref").target<bool (*)(emscripten::val)>();
            bool (*const* oldCallback)(emscripten::val) = oldVnode->hash & hasRef ? oldCallbacks.at("ref").target<bool (*)(emscripten::val)>() : nullptr;
            if (!callback || !oldCallback || *oldCallback != *callback) {
                if (oldVnode->hash & hasRef) {
                    oldCallbacks.at("ref")(emscripten::val::null());
                }
                callbacks.at("ref")(
                    emscripten::val::module_property("nodes")[vnode->elm]);
            }
        } else if (oldVnode->hash & hasRef) {
            oldCallbacks.at("ref")(emscripten::val::null());
        }
    }

}

void wasmdom::diff(VNode* __restrict__ const oldVnode, VNode* __restrict__ const vnode)
{
    const unsigned int vnodes = vnode->hash | oldVnode->hash;

    if (vnodes & hasAttrs)
        diffAttrs(oldVnode, vnode);
    if (vnodes & hasProps)
        diffProps(oldVnode, vnode);
    if (vnodes & hasCallbacks)
        diffCallbacks(oldVnode, vnode);
}
