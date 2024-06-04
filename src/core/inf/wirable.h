#pragma once

#include <map>
#include <vector>

#include "core/forwarding.h"
#include "core/collection/traits.h"

namespace ark {

class Wirable {
public:
    class WiringContext {
    public:
        WiringContext(Traits& components)
            : _components(components) {
        }

        template<typename T> sp<T> getComponent() const {
            return _components.get<T>();
        }

        template<typename T> sp<T> addComponent(sp<T> component) {
        }

    private:
        ~WiringContext() = default;

    private:
        Traits& _components;
        std::map<TypeId, Box> _component_builders;
    };

public:
    virtual ~Wirable() = default;

    virtual std::vector<std::pair<TypeId, Box>> onWire(const Traits& components) = 0;

};

}
