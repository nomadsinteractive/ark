#ifndef ARK_CORE_IMPL_DICTIONARY_DICTIONARY_BY_ATTRIBUTE_NAME_H_
#define ARK_CORE_IMPL_DICTIONARY_DICTIONARY_BY_ATTRIBUTE_NAME_H_

#include <map>

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "core/inf/dictionary.h"

namespace ark {

class DictionaryByAttributeName : public Dictionary<document> {
public:
    DictionaryByAttributeName(const document& doc, const String& attrName, const String& childName = String());

    virtual document get(const String& name) override;

private:
    std::map<String, document> _by_names;
};

}

#endif
