#include "core/impl/dictionary/dictionary_by_attribute_name.h"

#include "core/types/null.h"
#include "core/util/documents.h"
#include "core/util/strings.h"

namespace ark {

DictionaryByAttributeName::DictionaryByAttributeName(const document& doc, const String& attrName, const String& childName)
{
    for(const document& node : (childName.empty() ? doc->children() : doc->children(childName)))
    {
        const String name = Documents::getAttribute(node, attrName);
        if(name)
        {
            DCHECK_WARN(name.at(0) != '#', "Top level id \"%s\" is hard reference, which is legal but pointless", name.c_str());
            _by_names[Strings::stripReference(name)] = node;
        }
    }
}

document DictionaryByAttributeName::get(const String& name)
{
    return _by_names.find(name) == _by_names.end() ? nullptr : _by_names.at(name);
}

}
