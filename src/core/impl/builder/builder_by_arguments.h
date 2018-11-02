#ifndef ARK_CORE_IMPL_BUILDER_BUILDER_BY_ARGUMENTS_H_
#define ARK_CORE_IMPL_BUILDER_BUILDER_BY_ARGUMENTS_H_

#include "core/base/scope.h"
#include "core/base/string.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"
#include "core/util/identifier.h"

namespace ark {

template<typename T> class BuilderByArguments : public Builder<T> {
public:
    BuilderByArguments(BeanFactory& factory, const String& name)
        : _references(factory.references()), _name(name) {
    }
    BuilderByArguments(BeanFactory& factory, const Identifier& id)
        : _references(factory.references()), _name(id.arg()), _fallback(factory.createBuilderByValue<T>(id.toString(), false)) {
        DCHECK(id.isArg(), "Cannot build \"%s\" because it's not an argument", id.toString().c_str());
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
