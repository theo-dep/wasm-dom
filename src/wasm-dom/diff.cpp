#include "diff.hpp"

#include "vnode.hpp"

#include <emscripten.h>
#include <emscripten/val.h>

namespace wasmdom
{

    void diffAttrs(const VNode* __restrict__ const oldVnode, const VNode* __restrict__ const vnode)
    {
        const Attrs& oldAttrs = oldVnode->data.attrs;
        const Attrs& attrs = vnode->data.attrs;

        for (const auto& [key, _] : oldAttrs) {
            if (!attrs.contains(key)) {
                EM_ASM_({ Module.removeAttribute(
                              $0,
                              Module['UTF8ToString']($1)); }, vnode->elm, key.c_str());
            }
        }

        for (const auto& [key, val] : attrs) {
            if (!oldAttrs.contains(key) || oldAttrs.at(key) != val) {
                EM_ASM_({ Module.setAttribute(
                              $0,
                              Module['UTF8ToString']($1),
                              Module['UTF8ToString']($2)); }, vnode->elm, key.c_str(), val.c_str());
            }
        }
    }

    void diffProps(const VNode* __restrict__ const oldVnode, const VNode* __restrict__ const vnode)
    {
        const Props& oldProps = oldVnode->data.props;
        const Props& props = vnode->data.props;

        emscripten::val elm = emscripten::val::module_property("nodes")[vnode->elm];

        EM_ASM_({ Module['nodes'][$0]['asmDomRaws'] = []; }, vnode->elm);

        for (const auto& [key, _] : oldProps) {
            if (!props.contains(key)) {
                elm.set(key.c_str(), emscripten::val::undefined());
            }
        }

        for (const auto& [key, val] : props) {
            EM_ASM_({ Module['nodes'][$0]['asmDomRaws'].push(Module['UTF8ToString']($1)); }, vnode->elm, key.c_str());

            if (!oldProps.contains(key) ||
                !val.strictlyEquals(oldProps.at(key)) ||
                ((key == "value" || key == "checked") &&
                 !val.strictlyEquals(elm[key.c_str()]))) {
                elm.set(key.c_str(), val);
            }
        }
    }

    void diffCallbacks(const VNode* __restrict__ const oldVnode, const VNode* __restrict__ const vnode)
    {
        const Callbacks& oldCallbacks = oldVnode->data.callbacks;
        const Callbacks& callbacks = vnode->data.callbacks;

        for (const auto& [key, _] : oldCallbacks) {
            if (!callbacks.contains(key) && key != "ref") {
                EM_ASM_({
					var key = Module['UTF8ToString']($1).replace(/^on/, "");
					var elm = Module['nodes'][$0];
					elm.removeEventListener(
						key,
						Module['eventProxy'],
						false
					);
					delete elm['asmDomEvents'][key]; }, vnode->elm, key.c_str());
            }
        }

        EM_ASM_({
			var elm = Module['nodes'][$0];
			elm['asmDomVNode'] = $1;
			if (elm['asmDomEvents'] === undefined) {
				elm['asmDomEvents'] = {};
			} }, vnode->elm, reinterpret_cast<std::uintptr_t>(vnode));

        for (const auto& [key, val] : callbacks) {
            if (!oldCallbacks.contains(key) && key != "ref") {
                EM_ASM_({
					var key = Module['UTF8ToString']($1).replace(/^on/, "");
					var elm = Module['nodes'][$0];
					elm.addEventListener(
						key,
						Module['eventProxy'],
						false
					);
					elm['asmDomEvents'][key] = Module['eventProxy']; }, vnode->elm, key.c_str());
            }
        }

        if (vnode->hash & hasRef) {
            bool (*const* callback)(emscripten::val) = callbacks.at("ref").target<bool (*)(emscripten::val)>();
            bool (*const* oldCallback)(emscripten::val) = oldVnode->hash & hasRef ? oldCallbacks.at("ref").target<bool (*)(emscripten::val)>() : nullptr;
            if (!callback || !oldCallback || *oldCallback != *callback) {
                if (oldVnode->hash & hasRef) {
                    oldCallbacks.at("ref")(emscripten::val::null());
                }
                callbacks.at("ref")(emscripten::val::module_property("nodes")[vnode->elm]);
            }
        } else if (oldVnode->hash & hasRef) {
            oldCallbacks.at("ref")(emscripten::val::null());
        }
    }
}

void wasmdom::diff(const VNode* __restrict__ const oldVnode, const VNode* __restrict__ const vnode)
{
    const unsigned int vnodes = vnode->hash | oldVnode->hash;

    if (vnodes & hasAttrs)
        diffAttrs(oldVnode, vnode);
    if (vnodes & hasProps)
        diffProps(oldVnode, vnode);
    if (vnodes & hasCallbacks)
        diffCallbacks(oldVnode, vnode);
}
