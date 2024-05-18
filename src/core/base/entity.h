#pragma once

#include "core/base/api.h"
#include "core/collection/traits.h"
#include "core/types/shared_ptr.h"

#include "core/base/resource_loader.h"

namespace ark {

class ARK_API Entity {
public:
//  [[script::bindings::constructor]]
    Entity(sp<ResourceLoader> resourceLoader = nullptr);

    template<typename T> bool hasComponent() const {
        return _components.has<T>();
    }

    template<typename T, typename... Args> const sp<T>& addComponent(Args&&... args) {
        return _components.ensure<T>(std::forward<Args>(args)...);
    }

//  [[script::bindings::loader]]
    template<typename T> sp<T> loadComponent(const String& name, const Scope& args) {
        CHECK(_resource_loader, "An Entity without ResourceLoader can't load anything.");
        sp<T> cmp = _resource_loader->load<T>(name, args);
        _components.put(cmp);
        return cmp;
    }

//  [[script::bindings::auto]]
    bool hasComponent(TypeId typeId) const;
//  [[script::bindings::auto]]
    void addComponent(TypeId typeId, Box box);
//  [[script::bindings::auto]]
    Optional<Box> getComponent(TypeId typeId) const;

private:
    sp<ResourceLoader> _resource_loader;
    Traits _components;
};

}
