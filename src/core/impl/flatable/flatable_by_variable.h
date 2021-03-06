#ifndef ARK_CORE_IMPL_FLATABLE_FLATABLE_BY_VARIABLE_H_
#define ARK_CORE_IMPL_FLATABLE_FLATABLE_BY_VARIABLE_H_

#include "core/inf/input.h"
#include "core/inf/holder.h"
#include "core/inf/variable.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"
#include "core/util/holder_util.h"

namespace ark {

template<typename T, typename S = T> class FlatableByVariable : public Input, public Holder, Implements<FlatableByVariable<T, S>, Input, Holder> {
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

    virtual void traverse(const Visitor& visitor) override {
        HolderUtil::visit(_var, visitor);
    }

    const sp<VarType>& var() const {
        return _var;
    }

    virtual bool update(uint64_t timestamp) override {
        return _var->update(timestamp);
    }

private:
    sp<VarType> _var;

};

}

#endif
