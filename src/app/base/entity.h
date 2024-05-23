#pragma once

#include "core/base/api.h"
#include "core/collection/traits.h"
#include "core/types/shared_ptr.h"

#include "core/base/resource_loader.h"

namespace ark {

class ARK_API Entity {
public:
//  [[script::bindings::constructor]]
    Entity(Traits components = Traits());
    ~Entity();

    template<typename T> bool hasComponent() const {
        return _components.has<T>();
    }

    template<typename T> sp<T> addComponent(sp<T> cmp) {
        return _components.put(std::move(cmp));
    }

    template<typename T, typename... Args> sp<T> makeComponent(Args&&... args) {
        return _components.ensure<T>(std::forward<Args>(args)...);
    }

//  [[script::bindings::loader]]
    template<typename T> sp<T> loadComponent(const String& name, const Scope& args) {
        const sp<ResourceLoader>& resourceLoader = _components.get<ResourceLoader>();
        CHECK(resourceLoader, "An Entity doesn't have a ResourceLoader component can't load anything.");
        sp<T> cmp = resourceLoader->load<T>(name, args);
        _components.put(cmp);
        return cmp;
    }

//  [[script::bindings::property]]
    uintptr_t id() const;

//  [[script::bindings::auto]]
    void dispose();

//  [[script::bindings::property]]
    sp<RenderObject> renderObject() const;

//  [[script::bindings::auto]]
    bool hasComponent(TypeId typeId) const;
//  [[script::bindings::auto]]
    void addComponent(Box box);
//  [[script::bindings::auto]]
    Optional<Box> getComponent(TypeId typeId) const;

    const Traits& components() const;

public:
    struct Ref {
        Ref(Entity& entity);

        Entity& _entity;
        bool _discarded;
    };

private:
    sp<Ref> _ref;

    Traits _components;
};

}
