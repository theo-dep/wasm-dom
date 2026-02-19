#pragma once

#ifdef __EMSCRIPTEN__
#include <emscripten/val.h>
#endif

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

#ifdef __EMSCRIPTEN__
    template <typename T>
    concept ValAttribute = std::convertible_to<T, emscripten::val>;

    template <typename T>
    concept CallbackAttribute = requires(T f) {
        { f(std::declval<emscripten::val>()) } -> std::convertible_to<bool>;
    };

    template <typename T>
    concept EventCallbackAttribute = requires(T f) {
        { f(std::declval<emscripten::val>()) } -> std::same_as<void>;
    };

    struct Event
    {
        std::size_t e{};
        bool operator==(const Event&) const;
    };
    static inline constexpr Event onMount{ 0 };
    static inline constexpr Event onUpdate{ 1 };
    static inline constexpr Event onUnmount{ 2 };

    struct EventHash
    {
        std::size_t operator()(const Event& e) const;
    };

    template <typename T>
    concept EventAttribute = std::convertible_to<T, Event>;

    template <typename T>
    concept AttributeKey = Stringifiable<T> || EventAttribute<T>;

    template <typename T>
    concept AttributeValue = StringAttribute<T> || ValAttribute<T> || CallbackAttribute<T> || EventCallbackAttribute<T>;
#else
    template <typename T>
    concept AttributeKey = Stringifiable<T>;

    template <typename T>
    concept AttributeValue = StringAttribute<T>;
#endif

    using Attrs = std::unordered_map<std::string, std::string>;

#ifdef __EMSCRIPTEN__
    using Callback = std::function<bool(emscripten::val)>;
    using EventCallback = std::function<void(emscripten::val)>;

    using Props = std::unordered_map<std::string, emscripten::val>;
    using Callbacks = std::unordered_map<std::string, Callback>;
    using EventCallbacks = std::unordered_map<Event, EventCallback, EventHash>;
#endif

    struct VNodeAttributes
    {
        Attrs attrs;
#ifdef __EMSCRIPTEN__
        Props props;
        Callbacks callbacks;
        EventCallbacks eventCallbacks;
#endif

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
            if constexpr (StringAttribute<V>) {
                attributes.attrs.emplace(key, value);
            }
#ifdef __EMSCRIPTEN__
            else if constexpr (EventAttribute<K> && EventCallbackAttribute<V>) {
                attributes.eventCallbacks.emplace(key, value);
            } else if constexpr (ValAttribute<V>) {
                attributes.props.emplace(key, value);
            } else if constexpr (CallbackAttribute<V>) {
                attributes.callbacks.emplace(key, value);
            }
#endif
            else {
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

#ifndef WASMDOM_COVERAGE
#include "attribute.inl.hpp"
#endif
