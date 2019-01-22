#include "core/dom/dom_document.h"

#include "core/collection/iterable.h"
#include "core/dom/dom_attribute.h"
#include "core/types/null.h"
#include "core/types/shared_ptr.h"

namespace ark {

DOMDocument::DOMDocument(const String& name)
    : DOMElement(name)
{
}

const document& DOMDocument::getChild(const String& name) const
{
    auto iter = _children_by_name.find(name);
    if(iter != _children_by_name.end()) {
        const List<document>& list = iter->second;
        if(!list.isEmpty())
            return list.items().front();
    }
    return document::null();
}

const document& DOMDocument::ensureChild(const String& name) const
{
    const document& child = getChild(name);
    DCHECK(child, "Document has no child \"%s\"", name.c_str());
    return child;
}

void DOMDocument::addChild(const sp<DOMDocument>& doc)
{
    auto iter = _children_by_name.find(doc->name());
    if(iter != _children_by_name.end())
    {
        List<document>& list = iter->second;
        list.push_back(doc);
    }
    else
    {
        _children_by_name[doc->name()].push_back(doc);
    }
    _children.push_back(doc);
}

const List<document>& DOMDocument::children() const
{
    return _children;
}

List<document>& DOMDocument::children(const String& name)
{
    if(_children_by_name.find(name) == _children_by_name.end())
        return List<document>::emptyList();
    return _children_by_name.at(name);
}

void DOMDocument::clear()
{
    _children_by_name.clear();
    _children.clear();
}

}
