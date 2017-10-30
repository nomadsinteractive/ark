#ifndef ARK_CORE_IMPL_BUILDER_BUILDER_BY_INSTANCE_H_
#define ARK_CORE_IMPL_BUILDER_BUILDER_BY_INSTANCE_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class BuilderByInstance : public Builder<T> {
public:
    BuilderByInstance(const sp<T>& instance)
        : _instance(instance) {
    }

    virtual sp<T> build(const sp<Scope>& /*args*/) override {
        return _instance;
    }

private:
    sp<T> _instance;
};

}

#endif
