#ifndef ARK_CORE_IMPL_DICTIONARY_DIRECTORY_WITH_PREFIX_H_
#define ARK_CORE_IMPL_DICTIONARY_DIRECTORY_WITH_PREFIX_H_

#include "core/inf/dictionary.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class DictionaryWithPrefix : public Dictionary<T> {
public:
    DictionaryWithPrefix(const sp<Dictionary<T>>& delegate, const String& prefix)
        : _delegate(delegate), _prefix(prefix) {
    }

    virtual T get(const String& name) override {
        return _delegate->get(_prefix + name);
    }

private:
    sp<Dictionary<T>> _delegate;
    String _prefix;
};

}

#endif
