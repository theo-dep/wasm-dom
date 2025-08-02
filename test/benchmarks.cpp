#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "utils.hpp"

using namespace wasmdom;
using namespace wasmdom::dsl;

TEST_CASE("benchmark")
{
    BENCHMARK_ADVANCED("create")(Catch::Benchmark::Chronometer meter)
    {
        std::vector<VNode> storage(meter.runs(), nullptr);

        meter.measure([&](int i) {
            storage[i] =
                div(("foo", "foo"s),
                    ("bar", "bar"s),
                    ("baz", "baz"s))(
                    { div(("foo", "foo"s)),
                      div(("foo", "foo"s)),
                      div(("foo", "foo"s))(
                          { div(("foo", "foo"s)),
                            div(("foo", "foo"s)),
                            div(("foo", "foo"s)) }
                      ) }
                );
        });
    };

    BENCHMARK_ADVANCED("patch without changes")(Catch::Benchmark::Chronometer meter)
    {
        setupDom();

        const auto createVNode = [] {
            Children children;
            children.resize(100, nullptr);
            for (std::size_t i = 0; i < children.size(); ++i) {
                children[i] =
                    span(("e", std::to_string(i)))(
                        { span(("e", std::to_string(i - 3))) }
                    );
            }
            return div(
                ("foo", "foo"s),
                ("bar", "bar"s),
                ("baz", "baz"s)
            )(children);
        };

        VDom vdom(getRoot());
        vdom.patch(createVNode());

        std::vector<VNode> storage(meter.runs(), nullptr);
        for (std::size_t i = 0; i < storage.size(); ++i) {
            storage[i] = createVNode();
        }

        meter.measure([&](int i) {
            vdom.patch(storage[i]);
        });
    };

    BENCHMARK_ADVANCED("patch with changes")(Catch::Benchmark::Chronometer meter)
    {
        setupDom();

        const auto createVNode1 = [] {
            Children children;
            children.resize(100, nullptr);
            for (std::size_t i = 0; i < children.size(); ++i) {
                children[i] =
                    span(("e", std::to_string(i)))(
                        { span(("e", std::to_string(i - 1))) }
                    );
            }
            return div(
                ("foo", "foo"s),
                ("bar", "bar"s),
                ("baz", "baz"s)
            )(children);
        };
        const auto createVNode2 = [] {
            Children children;
            children.resize(100, nullptr);
            for (std::size_t i = 0; i < children.size(); ++i) {
                children[i] =
                    span(("e", "27"s))(
                        { span(("e", "27"s)) }
                    );
            }
            return div(
                ("foo", "foo"s),
                ("bar", "bar"s),
                ("baz", "baz"s)
            )(children);
        };

        VDom vdom(getRoot());
        vdom.patch(createVNode2());

        std::vector<VNode> storage(meter.runs(), nullptr);
        for (std::size_t i = 0; i < storage.size(); ++i) {
            storage[i] = (i % 2 == 0 ? createVNode1() : createVNode2());
        }

        meter.measure([&](int i) {
            vdom.patch(storage[i]);
        });
    };

    BENCHMARK_ADVANCED("patch with additions")(Catch::Benchmark::Chronometer meter)
    {
        setupDom();

        const auto createVNode1 = [] {
            Children children;
            children.resize(100, nullptr);
            for (std::size_t i = 0; i < children.size(); ++i) {
                children[i] = span()(
                    { span() }
                );
            }
            return div(
                ("foo", "foo"s),
                ("bar", "bar"s),
                ("baz", "baz"s)
            )(children);
        };
        const auto createVNode2 = [] {
            return div(
                ("foo", "foo"s),
                ("bar", "bar"s),
                ("baz", "baz"s)
            );
        };

        VDom vdom(getRoot());
        vdom.patch(createVNode2());

        std::vector<VNode> storage(meter.runs(), nullptr);
        for (std::size_t i = 0; i < storage.size(); ++i) {
            storage[i] = (i % 2 == 0 ? createVNode1() : createVNode2());
        }

        meter.measure([&](int i) {
            vdom.patch(storage[i]);
        });
    };
}
