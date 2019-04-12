#ifndef ARK_CORE_IMPL_FLATABLE_FLATABLE_BY_VARIABLE_H_
#define ARK_CORE_IMPL_FLATABLE_FLATABLE_BY_VARIABLE_H_

#include "core/inf/flatable.h"
#include "core/inf/variable.h"

#include "core/types/shared_ptr.h"

namespace ark {

template<typename T, typename S = T> class FlatableByVariable : public Flatable {
public:
    typedef Variable<S> VarType;

    FlatableByVariable(const sp<VarType>& var)
        : _var(var) {
    }

    virtual void flat(void* buf) override {
        *reinterpret_cast<T*>(buf) = static_cast<T>(_var->val());
    }

    virtual uint32_t size() override {
        return static_cast<uint32_t>(sizeof(T));
    }

    const sp<VarType>& var() const {
        return _var;
    }

private:
    sp<VarType> _var;

};

}

#endif
