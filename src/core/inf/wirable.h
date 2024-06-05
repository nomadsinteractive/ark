#pragma once

#include <map>

#include "core/forwarding.h"
#include "core/base/scope.h"
#include "core/collection/traits.h"
#include "core/inf/builder.h"

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
                return iter->second.template toPtr<Builder<T>>()->build(Scope());
            return nullptr;
        }

        template<typename T> void addComponent(sp<T> component) {
            _components.put(std::move(component));
        }

        template<typename T> void shareComponent(sp<T>& component) {
            if(sp<T> other = getComponent<T>())
                component = std::move(other);
            else
                addComponent(component);
        }

        template<typename T> void addComponentBuilder(sp<Builder<T>> componentBuilder) {
            _component_builders.insert_or_assign(Type<T>::id(), Box(std::move(componentBuilder)));
        }

    private:
        Traits& _components;
        std::map<TypeId, Box> _component_builders;
    };

public:
    virtual ~Wirable() = default;

    virtual TypeId onWire(WiringContext& context) = 0;

};

}
