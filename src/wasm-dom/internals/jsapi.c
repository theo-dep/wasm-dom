#include <emscripten/em_js.h>

typedef struct _EM_VAL* EM_VAL;

EM_JS(EM_VAL, createElement, (const char* name),
    { return Emval.toHandle(document.createElement(UTF8ToString(name))); })

EM_JS(EM_VAL, createElementNS, (const char* ns, const char* name),
    { return Emval.toHandle(document.createElementNS(UTF8ToString(ns), UTF8ToString(name))); })

EM_JS(EM_VAL, createTextNode, (const char* text),
    { return Emval.toHandle(document.createTextNode(UTF8ToString(text))); })

EM_JS(EM_VAL, createComment, (const char* comment),
    { return Emval.toHandle(document.createComment(UTF8ToString(comment))); })

EM_JS(EM_VAL, createDocumentFragment, (void),
    { return Emval.toHandle(document.createDocumentFragment()); })

EM_JS(void, insertBefore, (EM_VAL parentNode, EM_VAL newNode, EM_VAL referenceNode),
    { Emval.toValue(parentNode).insertBefore(Emval.toValue(newNode), Emval.toValue(referenceNode)); })

EM_JS(void, removeChild, (EM_VAL parentNode, EM_VAL child),
    { Emval.toValue(parentNode).removeChild(Emval.toValue(child)); })

EM_JS(void, appendChild, (EM_VAL parentNode, EM_VAL child),
    { Emval.toValue(parentNode).appendChild(Emval.toValue(child)); })

EM_JS(void, removeAttribute, (EM_VAL node, const char * attribute),
    { Emval.toValue(node).removeAttribute(UTF8ToString(attribute)); })

EM_JS(void, setAttributeNS, (EM_VAL node, const char* ns, const char * attribute, const char * value),
    { Emval.toValue(node).setAttributeNS(UTF8ToString(ns), UTF8ToString(attribute), UTF8ToString(value)); })

EM_JS(void, setAttribute, (EM_VAL node, const char * attribute, const char * value),
    { Emval.toValue(node).setAttribute(UTF8ToString(attribute), UTF8ToString(value)); })

EM_JS(void, addEventListener_, (EM_VAL node, const char * event, EM_VAL listener),
    { Emval.toValue(node).addEventListener(UTF8ToString(event), Emval.toValue(listener), false); })

EM_JS(void, removeEventListener_, (EM_VAL node, const char * event, EM_VAL listener),
    { Emval.toValue(node).removeEventListener(UTF8ToString(event), Emval.toValue(listener), false); })
