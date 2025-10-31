#pragma once

#include "core/inf/uploader.h"
#include "core/inf/variable.h"
#include "core/inf/writable.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T, typename S = T> class UploaderOfVariable final : public Uploader, Implements<UploaderOfVariable<T, S>, Uploader> {
public:
    typedef Variable<S> VarType;

    UploaderOfVariable(const sp<VarType>& var)
        : Uploader(sizeof(T)), _var(var) {
    }

    void upload(Writable& buf) override {
        buf.writeObject(static_cast<T>(_var->val()));
    }

    bool update(uint32_t tick) override {
        return _var->update(tick);
    }

private:
    sp<VarType> _var;
};

}
