#ifndef ARK_CORE_IMPL_DICTIONARY_DICTIONARY_IMPL_H_
#define ARK_CORE_IMPL_DICTIONARY_DICTIONARY_IMPL_H_

#include <map>

#include "core/inf/dictionary.h"

namespace ark {

template<typename T> class DictionaryImpl : public Dictionary<T> {
public:
    virtual T get(const String& name) override {
        const auto iter = _map.find(name);
        return iter == _map.end() ? T() : iter->second;
    }

private:
    std::map<String, T> _map;

};

}

#endif
