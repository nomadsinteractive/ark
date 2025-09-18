#include "core/dom/dom_document.h"

#include "core/dom/dom_attribute.h"
#include "core/types/shared_ptr.h"

namespace ark {

DOMDocument::DOMDocument(const String& name, const String& value, Table<String, attribute> attributes)
    : DOMElement(name, value, std::move(attributes))
{
}

document DOMDocument::getChild(const String& name) const
{
    if(const auto iter = _children_by_name.find(name); iter != _children_by_name.end())
    {
        const Vector<document>& list = iter->second;
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
    if(const auto iter = _children_by_name.find(doc->name()); iter != _children_by_name.end())
    {
        Vector<document>& list = iter->second;
        list.push_back(doc);
    }
    else
        _children_by_name[doc->name()].push_back(doc);
    _children.push_back(doc);
}

const Vector<document>& DOMDocument::children() const
{
    return _children;
}

const Vector<document>& DOMDocument::children(const String& name)
{
    static Vector<document> EMPTY;
    const auto iter = _children_by_name.find(name);
    if(iter == _children_by_name.end())
        return EMPTY;
    return iter->second;
}

}
