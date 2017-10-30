#ifndef ARK_CORE_IMPL_BUILDER_BUILDER_BY_ARGUMENTS_H_
#define ARK_CORE_IMPL_BUILDER_BUILDER_BY_ARGUMENTS_H_

#include "core/base/scope.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"

namespace ark {

template<typename T> class BuilderByArguments : public Builder<T> {
public:
    BuilderByArguments(const String& name, const WeakPtr<Scope>& references)
        : _name(name), _references(references) {
    }

    virtual sp<T> build(const sp<Scope>& args) override {
        const sp<Scope> reference = _references.lock();
        DCHECK(reference, "BeanFactory has been disposed");
        sp<T> value = args ? args->get<T>(_name) : nullptr;
        if(!value)
            value = reference->get<T>(_name);
        DCHECK(value, "Cannot get argument \"%s\"", _name.c_str());
        return value;
    }

private:
    String _name;

    WeakPtr<Scope> _references;

};

}

#endif
