#include <wasm-dom.hpp>

using namespace wasmdom;
using namespace wasmdom::dsl;

int runs = 100;
VDom vdom;

std::vector<VNode> noChangesStorage(runs, nullptr);
std::vector<VNode> changesStorage(runs, nullptr);
std::vector<VNode> additionStorage(runs, nullptr);

void end();

bool create(emscripten::val)
{
    for (int i = 0; i < runs; ++i) {
        VNode vnode =
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
    }

    end();
    return true;
}

void preparePatchWithoutChanges()
{
    for (VNode& vnode : noChangesStorage) {
        Children children;
        children.resize(100, nullptr);
        for (std::size_t i = 0; i < children.size(); ++i) {
            children[i] =
                span(("e", std::to_string(i)))(
                    { span(("e", std::to_string(i - 3))) }
                );
        }

        vnode =
            div(
                ("foo", "foo"s),
                ("bar", "bar"s),
                ("baz", "baz"s)
            )(children);
    }
}

bool patchWithoutChanges(emscripten::val)
{
    for (const VNode& vnode : noChangesStorage) {
        vdom.patch(vnode);
    }

    end();
    return true;
}

void preparePatchWithChanges()
{
    for (std::size_t i = 0; i < static_cast<std::size_t>(runs); ++i) {
        Children children;
        children.resize(100, nullptr);

        if (i % 2 == 0) {
            for (std::size_t j = 0; j < children.size(); ++j) {
                children[i] =
                    span(("e", std::to_string(j)))(
                        { span(("e", std::to_string(j - 1))) }
                    );
            }
        } else {
            for (VNode& child : children) {
                child =
                    span(("e", "27"s))(
                        { span(("e", "27"s)) }
                    );
            }
        }

        changesStorage[i] =
            div(
                ("foo", "foo"s),
                ("bar", "bar"s),
                ("baz", "baz"s)
            )(children);
    }
}

bool patchWithChanges(emscripten::val)
{
    for (const VNode& vnode : changesStorage) {
        vdom.patch(vnode);
    }

    end();
    return true;
}

void preparePatchWithAdditions()
{
    for (std::size_t i = 0; i < static_cast<std::size_t>(runs); ++i) {
        Children children;

        if (i % 2 == 0) {
            children.resize(100, nullptr);

            for (VNode& child : children) {
                child = span()(
                    { span() }
                );
            }
        }

        additionStorage[i] =
            div(
                ("foo", "foo"s),
                ("bar", "bar"s),
                ("baz", "baz"s)
            )(children);
    }
}

bool patchWithAdditions(emscripten::val)
{
    for (const VNode& vnode : additionStorage) {
        vdom.patch(vnode);
    }

    end();
    return true;
}

void render()
{
    VNode vnode =
        div()(
            { div()(a(("style", "cursor: pointer"s), ("onclick", f(create)))("Benchmark Creation")),
              div()(a(("style", "cursor: pointer"s), ("onclick", f(patchWithoutChanges)))("Benchmark Patch Without Changes")),
              div()(a(("style", "cursor: pointer"s), ("onclick", f(patchWithChanges)))("Benchmark Patch With Changes")),
              div()(a(("style", "cursor: pointer"s), ("onclick", f(patchWithAdditions)))("Benchmark Patch With Additions")) }
        );

    vdom.patch(vnode);
}

void end()
{
    static const auto onClickRender = [](emscripten::val) {
        render();
        return true;
    };
    VNode vnode = a(("style", "cursor: pointer"s), ("onclick", onClickRender))("Home");
    vdom.patch(vnode);
}

int main()
{
    vdom = VDom(
        emscripten::val::global("document").call<emscripten::val>("getElementById", std::string("root"))
    );

    preparePatchWithoutChanges();
    preparePatchWithChanges();
    preparePatchWithAdditions();

    render();
}
