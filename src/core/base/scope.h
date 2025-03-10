#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/inf/dictionary.h"
#include "core/types/box.h"
#include "core/types/optional.h"

namespace ark {

/*

class BoxBundle {
    [[script::bindings::getprop]]
    virtual Box get(const String& name) override;
};

*/

class ARK_API Scope final : public BoxBundle {
public:
    Scope() = default;
    Scope(Map<String, Box> variables);
    DEFAULT_COPY_AND_ASSIGN(Scope);

    Box get(const String& name) override;
    void put(const String& name, Box value);

    const Map<String, Box>& variables() const;

    Optional<Box> getObject(const String& name) const;

    template<typename T> Optional<T> getObject(const String& name) const {
        return getObject_sfinae<T>(name, nullptr);
    }

    template<typename T> void putObject(const String& name, T obj) {
        putObject_sfinae<T>(name, std::move(obj), nullptr);
    }

private:
    template<typename T> Optional<T> getObject_sfinae(const String& name, const typename T::_PtrType*) const {
        if(const Optional<Box> opt = getObject(name)) {
            sp<typename T::_PtrType> inst = opt->template as<typename T::_PtrType>();
            return inst || !opt.value() ? Optional<T>(std::move(inst)) : Optional<T>();
        }
        return {};
    }

    template<typename T> Optional<T> getObject_sfinae(const String& name, ...) const {
        if(const Optional<Box> box = getObject(name)) {
            sp<typename T::_PtrType> inst = box->template as<typename T::_PtrType>();
            return inst ? Optional<T>(*inst) : Optional<T>();
        }
        return {};
    }

    template<typename T> void putObject_sfinae(const String& name, T obj, const typename T::_PtrType*) {
        put(name, Box(std::move(obj)));
    }

    template<typename T> void putObject_sfinae(const String& name, T obj, ...) {
        put(name, Box(sp<T>::make(std::move(obj))));
    }

private:
    Map<String, Box> _variables;
};

}
