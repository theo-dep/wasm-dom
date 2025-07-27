#pragma once

#include <emscripten/val.h>

#include <concepts>
#include <functional>
#include <string>
#include <unordered_map>

namespace wasmdom
{

    template <typename T>
    concept Stringifiable = std::convertible_to<T, std::string>;

    template <typename T>
    concept StringAttribute = Stringifiable<T>;

    template <typename T>
    concept ValAttribute = std::convertible_to<T, emscripten::val>;

    template <typename T>
    concept CallbackAttribute = requires(T f) {
        { f(std::declval<emscripten::val>()) } -> std::convertible_to<bool>;
    };

    template <typename T>
    concept Attribute = StringAttribute<T> || ValAttribute<T> || CallbackAttribute<T>;

    using Callback = std::function<bool(emscripten::val)>;

    using Attrs = std::unordered_map<std::string, std::string>;
    using Props = std::unordered_map<std::string, emscripten::val>;
    using Callbacks = std::unordered_map<std::string, Callback>;

    struct VNodeAttributes
    {
        Attrs attrs;
        Props props;
        Callbacks callbacks;
    };

    template <Stringifiable K, Attribute V>
    using Pair = std::pair<K, V>;

    namespace detail
    {
        template <Stringifiable K, Attribute V>
        inline void attributeToVNode(VNodeAttributes& attributes, Pair<K, V>&& attribute)
        {
            auto&& [key, value]{ attribute };
            if constexpr (StringAttribute<V>) {
                attributes.attrs.emplace(key, value);
            } else if constexpr (ValAttribute<V>) {
                attributes.props.emplace(key, value);
            } else if constexpr (CallbackAttribute<V>) {
                attributes.callbacks.emplace(key, value);
            } else {
                static_assert(false, "Type not supported");
            }
        }
    }

    template <Stringifiable... K, Attribute... V>
    inline VNodeAttributes attributesToVNode(Pair<K, V>&&... attributes)
    {
        VNodeAttributes vnodeAttributes;
        (detail::attributeToVNode(vnodeAttributes, std::forward<Pair<K, V>>(attributes)), ...);
        return vnodeAttributes;
    }

}
