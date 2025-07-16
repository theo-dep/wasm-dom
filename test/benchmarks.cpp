#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "utils.hpp"

using namespace wasmdom;

TEST_CASE("benchmark")
{
    BENCHMARK_ADVANCED("create")(Catch::Benchmark::Chronometer meter)
    {
        std::vector<VNode*> storage(meter.runs());

        meter.measure([&](int i) {
            storage[i] =
                h("div",
                  Data(
                      Attrs{
                          { "foo", "foo" },
                          { "bar", "bar" },
                          { "baz", "baz" } }),
                  Children{
                      h("div", Data(Attrs{ { "foo", "foo" } })),
                      h("div", Data(Attrs{ { "foo", "foo" } })),
                      h("div", Data(Attrs{ { "foo", "foo" } }),
                        Children{
                            h("div", Data(Attrs{ { "foo", "foo" } })),
                            h("div", Data(Attrs{ { "foo", "foo" } })),
                            h("div", Data(Attrs{ { "foo", "foo" } })) }) });
        });

        for (const VNode* vnode : storage)
            deleteVNode(vnode);
    };

    BENCHMARK_ADVANCED("patch without changes")(Catch::Benchmark::Chronometer meter)
    {
        setupDom();

        const auto createVNode = [] {
            Children children;
            children.reserve(100);
            for (int i = 0; i < 100; ++i) {
                children.push_back(h("span",
                                     Data(
                                         Attrs{
                                             { "e", std::to_string(i) } }),
                                     Children{
                                         h("span",
                                           Data(
                                               Attrs{
                                                   { "e", std::to_string(i - 3) } })) }));
            }
            return h("div",
                     Data(
                         Attrs{
                             { "foo", "foo" },
                             { "bar", "bar" },
                             { "baz", "baz" } }),
                     children);
        };

        std::vector<std::pair<VNode*, VNode*>> storage(meter.runs());
        for (std::size_t i = 0; i < storage.size(); ++i) {
            auto& [vnode1, vnode2] = storage[i];

            if (i == 0) {
                vnode1 = patch(getRoot(), createVNode());
                vnode2 = createVNode();
            } else {
                vnode1 = storage[i - 1].second;
                vnode2 = createVNode();
            }
        }

        meter.measure([&](int i) {
            const auto& [vnode1, vnode2] = storage[i];

            patch(vnode1, vnode2);
        });

        deleteVNode(storage[storage.size() - 1].second);
    };

    BENCHMARK_ADVANCED("patch with changes")(Catch::Benchmark::Chronometer meter)
    {
        setupDom();

        const auto createVNode1 = [] {
            Children children;
            children.reserve(100);
            for (int i = 0; i < 100; ++i) {
                children.push_back(h("span",
                                     Data(
                                         Attrs{
                                             { "e", std::to_string(i) } }),
                                     Children{
                                         h("span",
                                           Data(
                                               Attrs{
                                                   { "e", std::to_string(i - 1) } })) }));
            }
            return h("div",
                     Data(
                         Attrs{
                             { "foo", "foo" },
                             { "bar", "bar" },
                             { "baz", "baz" } }),
                     children);
        };
        const auto createVNode2 = [] {
            Children children;
            children.reserve(100);
            for (int i = 0; i < 100; ++i) {
                children.push_back(h("span",
                                     Data(
                                         Attrs{
                                             { "e", "27" } }),
                                     Children{
                                         h("span",
                                           Data(
                                               Attrs{
                                                   { "e", "27" } })) }));
            }
            return h("div",
                     Data(
                         Attrs{
                             { "foo", "foo" },
                             { "bar", "bar" },
                             { "baz", "baz" } }),
                     children);
        };

        std::vector<std::pair<VNode*, VNode*>> storage(meter.runs());
        for (std::size_t i = 0; i < storage.size(); ++i) {
            auto& [vnode1, vnode2] = storage[i];

            if (i == 0) {
                vnode1 = patch(getRoot(), createVNode1());
                vnode2 = createVNode2();
            } else {
                vnode1 = storage[i - 1].second;
                vnode2 = (i % 2 == 0 ? createVNode2() : createVNode1());
            }
        }

        meter.measure([&](int i) {
            const auto& [vnode1, vnode2] = storage[i];

            patch(vnode1, vnode2);
        });

        deleteVNode(storage[storage.size() - 1].second);
    };

    BENCHMARK_ADVANCED("patch with additions")(Catch::Benchmark::Chronometer meter)
    {
        setupDom();

        const auto createVNode1 = [] {
            Children children;
            children.reserve(100);
            for (int i = 0; i < 100; ++i) {
                children.push_back(h("span",
                                     Children{
                                         h("span") }));
            }
            return h("div",
                     Data(
                         Attrs{
                             { "foo", "foo" },
                             { "bar", "bar" },
                             { "baz", "baz" } }),
                     children);
        };
        const auto createVNode2 = [] {
            return h("div",
                     Data(
                         Attrs{
                             { "foo", "foo" },
                             { "bar", "bar" },
                             { "baz", "baz" } }));
        };

        std::vector<std::pair<VNode*, VNode*>> storage(meter.runs());
        for (std::size_t i = 0; i < storage.size(); ++i) {
            auto& [vnode1, vnode2] = storage[i];

            if (i == 0) {
                vnode1 = patch(getRoot(), createVNode1());
                vnode2 = createVNode2();
            } else {
                vnode1 = storage[i - 1].second;
                vnode2 = (i % 2 == 0 ? createVNode2() : createVNode1());
            }
        }

        meter.measure([&](int i) {
            const auto& [vnode1, vnode2] = storage[i];

            patch(vnode1, vnode2);
        });

        deleteVNode(storage[storage.size() - 1].second);
    };
}
