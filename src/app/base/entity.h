#pragma once

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/forwarding.h"
#include "core/collection/traits.h"
#include "core/types/ref.h"
#include "core/types/shared_ptr.h"

#include "core/base/resource_loader.h"

namespace ark {

//[[script::bindings::debris]]
class ARK_API Entity final : public Debris {
public:
    struct Component {
        Box _trait;
        document _manifest;
    };

//  [[script::bindings::constructor]]
    Entity(Traits components = {});
    Entity(Vector<Component> components);
    ~Entity() override;

    void traverse(const Visitor& visitor) override;

    template<typename T> bool hasComponent() const {
        return _components.has<T>();
    }

    template<typename T> void addComponent(sp<T> cmp) {
        addComponent(Box(std::move(cmp)));
    }

//  [[script::bindings::property]]
    const sp<Ref>& ref() const;
//  [[script::bindings::auto]]
    void discard();
//  [[script::bindings::auto]]
    bool hasComponent(TypeId typeId) const;
//  [[script::bindings::auto]]
    void addComponent(Box component);
//  [[script::bindings::auto]]
    Optional<Box> getComponent(TypeId typeId) const;

//  [[script::bindings::property]]
    Box tag() const;

    const Traits& components() const;

//  [[plugin::builder]]
    class BUILDER final : public Builder<Entity> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Entity> build(const Scope& args) override;

    private:
        struct ComponentBuilder;

    private:
        sp<Builder<Vector<Box>>> _boxes;
        Vector<ComponentBuilder> _components;
    };

private:
    void preWire();

private:
    sp<Ref> _ref;
    Traits _components;
};

}
