#ifndef ARK_CORE_IMPL_DICTIONARY_DICTIONARY_IMPL_H_
#define ARK_CORE_IMPL_DICTIONARY_DICTIONARY_IMPL_H_

#include <map>

#include "core/inf/dictionary.h"
#include "core/types/null.h"

namespace ark {

template<typename T> class DictionaryImpl : public Dictionary<T> {
public:
    virtual T get(const String& name) override {
        return _map.find(name) == _map.end() ? Null::ptr<T>() : _map.at(name);
    }

private:
    std::map<String, T> _map;

};

}

#endif
