#include "core/dom/dom_document.h"

#include "core/collection/iterable.h"
#include "core/dom/dom_attribute.h"
#include "core/types/null.h"
#include "core/types/shared_ptr.h"

namespace ark {

DOMDocument::DOMDocument(const String& name, const String& value, ElementType type)
    : DOMElement(name, value, type)
{
}

document DOMDocument::getChild(const String& name) const
{
    auto iter = _children_by_name.find(name);
    if(iter != _children_by_name.end()) {
        const std::vector<document>& list = iter->second;
        if(!list.empty())
            return list.front();
    }
    return nullptr;
}

document DOMDocument::ensureChild(const String& name) const
{
    document child = getChild(name);
    DCHECK(child, "Document has no child \"%s\"", name.c_str());
    return child;
}

void DOMDocument::addChild(const sp<DOMDocument>& doc)
{
    auto iter = _children_by_name.find(doc->name());
    if(iter != _children_by_name.end())
    {
        std::vector<document>& list = iter->second;
        list.push_back(doc);
    }
    else
    {
        _children_by_name[doc->name()].push_back(doc);
    }
    _children.push_back(doc);
}

const std::vector<document>& DOMDocument::children() const
{
    return _children;
}

const std::vector<document>& DOMDocument::children(const String& name)
{
    static std::vector<document> EMPTY;
    const auto iter = _children_by_name.find(name);
    if(iter == _children_by_name.end())
        return EMPTY;
    return iter->second;
}

}
