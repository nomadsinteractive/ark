#ifndef ARK_CORE_IMPL_BUILDER_BUILDER_BY_SOFT_REF_H_
#define ARK_CORE_IMPL_BUILDER_BUILDER_BY_SOFT_REF_H_

#include "core/base/scope.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"

namespace ark {

template<typename T> class BuilderBySoftRef : public Builder<T> {
public:
    BuilderBySoftRef(const String& name, const WeakPtr<Scope>& references, const sp<Builder<T>>& delegate)
        : _name(name), _references(references), _delegate(delegate) {
    }

    virtual sp<T> build(const sp<Scope>& args) override {
        const sp<Scope> reference = _references.lock();
        DCHECK(reference, "BeanFactory has been disposed");
        sp<T> inst = reference->get<T>(_name);
        if(!inst) {
            inst = _delegate->build(args);
            NOT_NULL(inst);
            reference->put<T>(_name, inst);
            _delegate = nullptr;
        }
        return inst;
    }

private:
    String _name;
    WeakPtr<Scope> _references;
    sp<Builder<T>> _delegate;
};

}

#endif
