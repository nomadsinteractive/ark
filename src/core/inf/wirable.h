#pragma once

#include "core/forwarding.h"
#include "core/base/bean_factory.h"
#include "core/base/scope.h"
#include "core/collection/traits.h"
#include "core/inf/builder.h"
#include "core/util/documents.h"

namespace ark {

class Wirable {
public:
    class WiringContext {
    public:
        WiringContext(Traits& components)
            : _components(components) {
        }

        template<typename T> sp<T> getComponent() const {
            if(sp<T> component = _components.get<T>())
                return component;
            if(const auto iter = _component_builders.find(Type<T>::id()); iter != _component_builders.end())
                return iter->second.template toPtr<Builder<T>>()->build({});
            return nullptr;
        }

        template<typename T> sp<T> ensureComponent() const {
            sp<T> component = _components.get<T>();
            CHECK(component, "Component \"%s\" does not exist", Class::ensureClass<T>()->name());
            return component;
        }

        template<typename T> void putComponent(sp<T> component) {
            CHECK_WARN(!_components.has<T>(), "Overriding component: \"%s\"", Class::ensureClass<T>()->name());
            _components.put(std::move(component));
        }
        template<typename T> void addComponent(sp<T> component) {
            _components.add(std::move(component));
        }

        template<typename T> void setComponentBuilder(sp<Builder<T>> componentBuilder) {
            _component_builders.insert_or_assign(Type<T>::id(), Box(std::move(componentBuilder)));
        }

    private:
        Traits& _components;
        Map<TypeId, Box> _component_builders;
    };

    virtual ~Wirable() = default;

    virtual TypeId onPoll(WiringContext& context) = 0;
    virtual void onWire(const WiringContext& context) = 0;

};

}
