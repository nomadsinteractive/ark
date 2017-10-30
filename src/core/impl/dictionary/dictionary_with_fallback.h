#ifndef ARK_CORE_IMPL_DICTIONARY_DICTIONARY_WITH_FALLBACK_H_
#define ARK_CORE_IMPL_DICTIONARY_DICTIONARY_WITH_FALLBACK_H_

#include "core/inf/dictionary.h"

namespace ark {

template<typename T> class DictionaryWithFallback : public Dictionary<T> {
public:
    DictionaryWithFallback(const sp<Dictionary<T>>& delegate, const sp<Dictionary<T>>& extension)
        : _delegate(delegate), _extension(extension) {
    }

    virtual T get(const String& name) override {
        const T v = _delegate->get(name);
        return v ? v : _extension->get(name);
    }

    const sp<Dictionary<T>>& delegate() const {
        return _delegate;
    }

    const sp<Dictionary<T>>& extension() const {
        return _extension;
    }

private:
    sp<Dictionary<T>> _delegate;
    sp<Dictionary<T>> _extension;

};

}

#endif
