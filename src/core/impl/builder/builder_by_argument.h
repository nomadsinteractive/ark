#ifndef ARK_CORE_IMPL_BUILDER_BUILDER_BY_ARGUMENT_H_
#define ARK_CORE_IMPL_BUILDER_BUILDER_BY_ARGUMENT_H_

#include "core/base/scope.h"
#include "core/base/string.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"
#include "core/base/identifier.h"

namespace ark {

template<typename T> class BuilderByArgument : public Builder<T> {
public:
    BuilderByArgument(const WeakPtr<Scope>& references, const String& name, const sp<Builder<T>> fallback = nullptr)
        : _references(references), _name(name), _fallback(std::move(fallback)) {
    }

    virtual sp<T> build(const sp<Scope>& args) override {
        sp<T> value = args ? args->get<T>(_name) : nullptr;
        if(!value) {
            const sp<Scope> references = _references.lock();
            DCHECK(references, "BeanFactory has been disposed");
            value = references->get<T>(_name);
        }
        DCHECK(value || _fallback, "Cannot get argument \"%s\"", _name.c_str());
        return value ? value : _fallback->build(args);
    }

private:
    WeakPtr<Scope> _references;
    String _name;
    sp<Builder<T>> _fallback;
};

}

#endif
