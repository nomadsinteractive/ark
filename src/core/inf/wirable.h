#pragma once

#include <map>

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
                return iter->second.template toPtr<Builder<T>>()->build(Scope());
            return nullptr;
        }

        template<typename T> void addComponent(sp<T> component) {
            if(sp<T> existing = getComponent<T>()) {
                sp<Vector<T>> vector = _components.get<Vector<T>>();
                if(!vector) {
                    vector = sp<Vector<T>>::make();
                    vector->push_back(std::move(existing));
                    _components.put(vector);
                }
                vector->push_back(component);
            }
            _components.put(std::move(component));
        }

        template<typename T> void setComponentBuilder(sp<Builder<T>> componentBuilder) {
            _component_builders.insert_or_assign(Type<T>::id(), Box(std::move(componentBuilder)));
        }

    private:
        Traits& _components;
        std::map<TypeId, Box> _component_builders;
    };

    virtual ~Wirable() = default;

    virtual TypeId onPoll(WiringContext& context) = 0;
    virtual void onWire(const WiringContext& context) = 0;

};

}
