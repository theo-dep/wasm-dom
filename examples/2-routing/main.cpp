#include <wasm-dom.hpp>

#include <emscripten.h>
#include <emscripten/bind.h>

#include <functional>
#include <memory>
#include <unordered_map>

using namespace wasmdom;
using namespace wasmdom::dsl;

bool handleHomeClick(const emscripten::val& e);
bool handleAboutClick(const emscripten::val& e);
bool handleContactClick(const emscripten::val& e);

std::unique_ptr<VDom> vdom = nullptr;

VNode root()
{
    return div(("id", "root"s), ("style", "flex-direction: column;"s));
}

VNode createNav(const Children& children)
{
    Children newChildren;
    newChildren.reserve(children.size());
    if (!children.empty()) {
        newChildren.push_back(children[0]);

        if (children.size() > 1) {
            for (auto it = std::next(children.cbegin()); it != children.cend(); ++it) {
                newChildren.push_back(t("|"));
                newChildren.push_back(*it);
            }
        }
    }
    return div()(newChildren);
}

VNode createButton(const std::string& route, bool (*callback)(const emscripten::val&), const std::string& text)
{
    return a(("class", "button"s), ("href", route), ("onclick", f(callback)))(text);
}

class Router
{
private:
    std::unordered_map<std::string, std::function<VNode()>> _routes;

public:
    Router()
    {
        // Listen for popstate events (back/forward buttons)
        emscripten::val::global("window").call<void>("addEventListener", emscripten::val("popstate"), emscripten::val::module_property("onPopState"));
    }

    void navigate(const std::string& path)
    {
        // Update browser history
        emscripten::val::global("window")["history"].call<void>("pushState", emscripten::val::null(), emscripten::val(""), emscripten::val(path));
        render();
    }

    void render()
    {
        std::string path = emscripten::val::global("window")["location"]["pathname"].as<std::string>();
        if (path.empty())
            path = "/";

        if (_routes.contains(path)) {
            vdom->patch(_routes.at(path)());
            emscripten::val::global("console").call<void>("log", "Render to " + path);
        } else {
            static const VNode notFound = root()(
                { h1()("404 - Page Not Found"),
                  createButton("/", handleHomeClick, "Go Home") }
            );
            vdom->patch(notFound);
        }
    }

    void addRoute(const std::string& path, std::function<VNode()> component)
    {
        _routes.insert({ path, component });
    }
};

// Global router instance
std::unique_ptr<Router> router = nullptr;

// Handle browser back/forward buttons
EMSCRIPTEN_BINDINGS(routerBindings)
{
    emscripten::function("onPopState", emscripten::optional_override([](emscripten::val) {
                             // Handle hash change
                             if (router) {
                                 router->render();
                             }
                         }));
}

// Route components
VNode homePage()
{
    return root()(
        { h1()("Welcome to Home Page"),
          createNav(
              { createButton("/about", handleAboutClick, "About"),
                createButton("/contact", handleContactClick, "Contact") }
          ) }
    );
}

VNode aboutPage()
{
    return root()(
        { h1()("About Us"),
          p()("This is the about page."),
          createNav(
              { createButton("/", handleHomeClick, "Home"),
                createButton("/contact", handleContactClick, "Contact") }
          ) }
    );
}

VNode contactPage()
{
    return root()(
        { h1()("Contact Us"),
          p()("Get in touch with us."),
          createNav(
              { createButton("/", handleHomeClick, "Home"),
                createButton("/about", handleAboutClick, "About") }
          ) }
    );
}

// Event handlers
bool handleHomeClick(const emscripten::val& e)
{
    e.call<void>("preventDefault");
    router->navigate("/");
    return true;
}

bool handleAboutClick(const emscripten::val& e)
{
    e.call<void>("preventDefault");
    router->navigate("/about");
    return true;
}

bool handleContactClick(const emscripten::val& e)
{
    e.call<void>("preventDefault");
    router->navigate("/contact");
    return true;
}

int main()
{
    init();

    vdom = std::make_unique<wasmdom::VDom>(
        emscripten::val::global("document").call<emscripten::val>("getElementById", std::string("root"))
    );

    // Initialize router
    router = std::make_unique<Router>();
    router->addRoute("/", homePage);
    router->addRoute("/about", aboutPage);
    router->addRoute("/contact", contactPage);

    // Render initial route
    router->navigate("/");

    return 0;
}
