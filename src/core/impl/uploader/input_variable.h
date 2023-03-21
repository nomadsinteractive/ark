#pragma once

#include "core/inf/uploader.h"
#include "core/inf/holder.h"
#include "core/inf/variable.h"
#include "core/inf/writable.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"
#include "core/util/holder_util.h"

namespace ark {

template<typename T, typename S = T> class InputVariable : public Uploader, public Holder, Implements<InputVariable<T, S>, Uploader, Holder> {
public:
    typedef Variable<S> VarType;

    InputVariable(const sp<VarType>& var)
        : Uploader(sizeof(T)), _var(var) {
    }

    virtual void upload(Writable& buf) override {
        buf.writeObject(_var->val());
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
