#ifndef ARK_GRAPHICS_IMPL_VEC_VEC_WITH_TRANSLATION_H_
#define ARK_GRAPHICS_IMPL_VEC_VEC_WITH_TRANSLATION_H_

#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

template<typename T> class VecWithTranslation : public Variable<T> {
public:
    VecWithTranslation(const sp<Variable<T>>& delegate, const T& translation)
        : _delegate(delegate), _translation(translation) {
    }

    virtual T val() override {
        return _delegate->val() + _translation;;
    }

private:
    const sp<Variable<T>> _delegate;
    const T _translation;
};

}

#endif
