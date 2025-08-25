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

    struct Event
    {
        std::size_t e{};
        bool operator==(const Event&) const = default;
    };
    static inline constexpr Event onMount{ 0 };
    static inline constexpr Event onUpdate{ 1 };
    static inline constexpr Event onUnmount{ 2 };

    struct EventHash
    {
        inline std::size_t operator()(const Event& e) const
        {
            return std::hash<std::size_t>{}(e.e);
        }
    };

    template <typename T>
    concept EventAttribute = std::convertible_to<T, Event>;

    template <typename T>
    concept AttributeKey = Stringifiable<T> || EventAttribute<T>;

    template <typename T>
    concept AttributeValue = StringAttribute<T> || ValAttribute<T> || CallbackAttribute<T>;

    using Callback = std::function<bool(emscripten::val)>;

    using Attrs = std::unordered_map<std::string, std::string>;
    using Props = std::unordered_map<std::string, emscripten::val>;
    using Callbacks = std::unordered_map<std::string, Callback>;
    using EventCallbacks = std::unordered_map<Event, Callback, EventHash>;

    struct VNodeAttributes
    {
        Attrs attrs;
        Props props;
        Callbacks callbacks;
        EventCallbacks eventCallbacks;

#ifdef WASMDOM_COVERAGE
        VNodeAttributes();
        VNodeAttributes(const VNodeAttributes& other);
        VNodeAttributes(VNodeAttributes&& other);
        VNodeAttributes& operator=(const VNodeAttributes& other);
        VNodeAttributes& operator=(VNodeAttributes&& other);
        ~VNodeAttributes();
#endif
    };

    namespace internals
    {
        template <AttributeKey K, AttributeValue V>
        inline void attributeToVNode(VNodeAttributes& attributes, std::pair<K, V>&& attribute)
        {
            auto&& [key, value]{ attribute };
            if constexpr (EventAttribute<K> && CallbackAttribute<V>) {
                attributes.eventCallbacks.emplace(key, value);
            } else if constexpr (StringAttribute<V>) {
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

    template <AttributeKey... K, AttributeValue... V>
    inline VNodeAttributes attributesToVNode(std::pair<K, V>&&... attributes)
    {
        VNodeAttributes vnodeAttributes;
        (internals::attributeToVNode(vnodeAttributes, std::forward<std::pair<K, V>>(attributes)), ...);
        return vnodeAttributes;
    }

}
