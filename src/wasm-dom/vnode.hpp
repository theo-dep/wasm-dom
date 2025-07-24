#pragma once

#include <emscripten/val.h>

#include <atomic>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace wasmdom
{

    using Callback = std::function<bool(emscripten::val)>;
    using Attrs = std::unordered_map<std::string, std::string>;
    using Props = std::unordered_map<std::string, emscripten::val>;
    using Callbacks = std::unordered_map<std::string, Callback>;

    class VNode;
    using Children = std::vector<VNode>;

    enum VNodeFlags
    {
        // NodeType
        isElement = 1,
        isText = 1 << 1,
        isComment = 1 << 2,
        isFragment = 1 << 3,

        // flags
        hasKey = 1 << 4,
        hasText = 1 << 5,
        hasAttrs = 1 << 6,
        hasProps = 1 << 7,
        hasCallbacks = 1 << 8,
        hasDirectChildren = 1 << 9,
        hasChildren = hasDirectChildren | hasText,
        hasRef = 1 << 10,
        hasNS = 1 << 11,
        isNormalized = 1 << 12,

        // masks
        isElementOrFragment = isElement | isFragment,
        nodeType = isElement | isText | isComment | isFragment,
        removeNodeType = ~0 ^ nodeType,
        extractSel = ~0 << 13,
        id = extractSel | hasKey | nodeType
    };

    struct Data
    {
        Data() {}
        Data(const Attrs& dataAttrs,
             const Props& dataProps = Props(),
             const Callbacks& dataCallbacks = Callbacks())
            : attrs(dataAttrs)
            , props(dataProps)
            , callbacks(dataCallbacks)
        {
        }
        Data(const Attrs& dataAttrs,
             const Callbacks& dataCallbacks)
            : attrs(dataAttrs)
            , callbacks(dataCallbacks)
        {
        }
        Data(const Props& dataProps,
             const Callbacks& dataCallbacks = Callbacks())
            : props(dataProps)
            , callbacks(dataCallbacks)
        {
        }
        Data(const Callbacks& dataCallbacks)
            : callbacks(dataCallbacks)
        {
        }

        Attrs attrs;
        Props props;
        Callbacks callbacks;
    };

    class VNode
    {
        // SharedData and SharedDataPointer are inspired from QSharedData
        // https://codebrowser.dev/qt6/qtbase/src/corelib/tools/qshareddata.h.html

        struct SharedData
        {
            std::atomic_int8_t ref = 0;

            std::string sel;
            std::string key;
            std::string ns;
            unsigned int hash = 0;
            Data data;
            int elm = 0;
            Children children;
        };

        // https://codebrowser.dev/qt6/qtbase/src/corelib/thread/qatomic_cxx11.h.html#_ZN10QAtomicOps3refERSt6atomicITL0__E
        template <typename T>
        static inline bool ref(std::atomic<T>& ref)
        {
            /* Conceptually, we want to
             *    return ++ref != 0;
             * However, that would be sequentially consistent, and thus stronger
             * than what we need. Based on
             * http://eel.is/c++draft/atomics.types.memop#6, we know that
             * pre-increment is equivalent to fetch_add(1) + 1. Unlike
             * pre-increment, fetch_add takes a memory order argument, so we can get
             * the desired acquire-release semantics.
             * One last gotcha is that fetch_add(1) + 1 would need to be converted
             * back to T, because it's susceptible to integer promotion. To sidestep
             * this issue and to avoid UB on signed overflow, we rewrite the
             * expression to:
             */
            return ref.fetch_add(1, std::memory_order_acq_rel) != T(-1);
        }

        template <typename T>
        static inline bool deref(std::atomic<T>& ref)
        {
            // compare with ref
            return ref.fetch_sub(1, std::memory_order_acq_rel) != T(1);
        }

    public:
        VNode(std::nullptr_t) {}
        VNode(const std::string& nodeSel)
            : _data(new SharedData)
        {
            ref(_data->ref);
            _data->sel = nodeSel;
        }
        VNode(const std::string& nodeSel,
              const std::string& nodeText)
            : VNode(nodeSel)
        {
            normalize();
            if (_data->hash & isComment) {
                _data->sel = nodeText;
            } else {
                _data->children.emplace_back(nodeText, true);
                _data->hash |= hasText;
            }
        }
        VNode(const std::string& nodeText,
              bool textNode)
            : _data(new SharedData)
        {
            ref(_data->ref);
            if (textNode) {
                normalize();
                _data->sel = nodeText;
                // replace current type with text type
                _data->hash = (_data->hash & removeNodeType) | isText;
            } else {
                _data->sel = nodeText;
                normalize();
            }
        }
        VNode(const std::string& nodeSel,
              const Data& nodeData)
            : VNode(nodeSel)
        {
            _data->data = nodeData;
        }
        VNode(const std::string& nodeSel,
              const Children& nodeChildren)
            : VNode(nodeSel)
        {
            _data->children = nodeChildren;
        }
        VNode(const std::string& nodeSel,
              const VNode& child)
            : VNode(nodeSel)
        {
            _data->children.push_back(child);
        }
        VNode(const std::string& nodeSel,
              const Data& nodeData,
              const std::string& nodeText)
            : VNode(nodeSel, nodeData)
        {
            normalize();
            if (_data->hash & isComment) {
                _data->sel = nodeText;
            } else {
                _data->children.emplace_back(nodeText, true);
                _data->hash |= hasText;
            }
        }
        VNode(const std::string& nodeSel,
              const Data& nodeData,
              const Children& nodeChildren)
            : VNode(nodeSel, nodeData)
        {
            _data->children = nodeChildren;
        }
        VNode(const std::string& nodeSel,
              const Data& nodeData,
              const VNode& child)
            : VNode(nodeSel, nodeData)
        {
            _data->children.push_back(child);
        }

        ~VNode()
        {
            if (_data && !deref(_data->ref))
                delete _data;
        }
        [[nodiscard]] VNode(const VNode& other)
            : _data(other._data)
        {
            if (_data)
                ref(_data->ref);
        }
        VNode& operator=(const VNode& other)
        {
            if (other._data != _data) {
                if (other._data)
                    ref(other._data->ref);
                SharedData* old = std::exchange(_data, other._data);
                if (old && !deref(old->ref))
                    delete old;
            }
            return *this;
        }
        [[nodiscard]] VNode(VNode&& other)
            : _data(std::exchange(other._data, nullptr))
        {
        }
        VNode& operator=(VNode&& other)
        {
            VNode moved(std::move(other));
            std::swap(_data, moved._data);
            return *this;
        }

        const Attrs& attrs() const { return _data->data.attrs; }
        const Props& props() const { return _data->data.props; }
        const Callbacks& callbacks() const { return _data->data.callbacks; }

        const std::string& sel() const { return _data->sel; }
        const std::string& key() const { return _data->key; }
        const std::string& ns() const { return _data->ns; }
        unsigned int hash() const { return _data->hash; }
        int elm() const { return _data->elm; }

        void setElm(int nodeElm) { _data->elm = nodeElm; }

        const Children& children() const { return _data->children; }

        void normalize() { normalize(false); }

        std::string toHTML() const;

        void diff(const VNode& other) const;

        operator bool() const { return _data != nullptr; }
        bool operator!() const { return _data == nullptr; }
        bool operator==(const VNode& other) const { return _data == other._data; }
        bool operator==(std::nullptr_t) const { return _data == nullptr; }

        static VNode toVNode(const emscripten::val& node);

    private:
        void normalize(bool injectSvgNamespace);

        friend void diffCallbacks(const VNode& oldVnode, const VNode& vnode);
        friend emscripten::val functionCallback(const std::uintptr_t& sharedData, std::string callback, emscripten::val event);

        friend int createElm(VNode& vnode);
        friend void patchVNode(VNode& oldVnode, VNode& vnode, int parentElm);

        // contains selector for elements and fragments, text for comments and textNodes
        SharedData* _data = nullptr;
    };

}
