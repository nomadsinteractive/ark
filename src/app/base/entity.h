#pragma once

#include "core/base/api.h"
#include "core/inf/wirable.h"
#include "core/forwarding.h"
#include "core/collection/traits.h"
#include "core/types/ref.h"
#include "core/types/shared_ptr.h"

#include "core/base/resource_loader.h"

namespace ark {

//[[script::bindings::holder]]
class ARK_API Entity : public Holder {
public:
//  [[script::bindings::constructor]]
    Entity(Traits components = Traits());
    ~Entity() override;

    void traverse(const Visitor& visitor) override;

    template<typename T> bool hasComponent() const {
        return _components.has<T>();
    }

    template<typename T> void addComponent(sp<T> cmp) {
        addComponent(Box(std::move(cmp)));
    }

//  [[script::bindings::loader]]
    template<typename T> sp<T> loadComponent(const String& name, const Scope& args) {
        const sp<ResourceLoader>& resourceLoader = _components.get<ResourceLoader>();
        CHECK(resourceLoader, "An Entity doesn't have a ResourceLoader component can't load anything.");
        sp<T> cmp = resourceLoader->load<T>(name, args);
        addComponent<T>(cmp);
        return cmp;
    }

//  [[script::bindings::property]]
    sp<EntityId> id() const;

//  [[script::bindings::auto]]
    void dispose();

//  [[script::bindings::auto]]
    bool hasComponent(TypeId typeId) const;
//  [[script::bindings::auto]]
    void addComponent(Box box);
//  [[script::bindings::auto]]
    Optional<Box> getComponent(TypeId typeId) const;

    const Traits& components() const;

private:
    void doWire();

private:
    sp<EntityRef> _ref;
    Traits _components;

    friend class EntityId;
};

}
