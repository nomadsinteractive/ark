#pragma once

#include "core/forwarding.h"
#include "core/base/constants.h"
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
        ~WiringContext() {
            for(Box& i : _intermedia_list)
                _components.add(i.typeId(), std::move(i));

            for(auto& [k, v] : _intermedia_map)
                _components.put(k, std::move(v));
        }

        template<typename T> sp<T> getComponent() const {
            if(sp<T> component = _components.get<T>())
                return component;
            if(const auto iter = _intermedia_map.find(Type<T>::id()); iter != _intermedia_map.end())
                return iter->second.template toPtr<T>();
            return nullptr;
        }

        template<typename T> sp<T> ensureComponent() const {
            sp<T> component = getComponent<T>();
            CHECK(component, "Component \"%s\" does not exist", Class::ensureClass<T>()->name());
            return component;
        }

        template<typename T> void setComponent(sp<T> component) {
            CHECK_WARN(!(_components.has<T>() || _intermedia_map.find(Type<T>::id()) != _intermedia_map.end()), "Overriding component: \"%s\"", Class::ensureClass<T>()->name());
            setIntermediaComponent(std::move(component));
        }

        template<typename T> void addComponent(sp<T> component) {
            setIntermediaComponent(component);
            _intermedia_list.emplace_back(std::move(component));
        }

    private:
        template<typename T> void setIntermediaComponent(sp<T> component) {
            if(!_components.has<T>())
                _intermedia_map.insert_or_assign(Type<T>::id(), Box(std::move(component)));
        }

    private:
        Traits& _components;
        Vector<Box> _intermedia_list;
        Map<TypeId, Box> _intermedia_map;
    };

    virtual ~Wirable() = default;

    virtual TypeId onPoll(WiringContext& context) {
        return constants::TYPE_ID_NONE;
    }

    virtual void onWire(const WiringContext& context) = 0;

};

}
