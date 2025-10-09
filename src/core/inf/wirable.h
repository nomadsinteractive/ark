#pragma once

#include "core/forwarding.h"
#include "core/base/constants.h"
#include "core/collection/traits.h"

namespace ark {

class Wirable {
public:
    class WiringContext {
    public:
        WiringContext(Traits& components, const bool uploadOnClose = true)
            : _components(components), _upload_on_close(uploadOnClose) {
        }
        ~WiringContext() {
            if(_upload_on_close) {
                for(auto& [k, v] : _component_map)
                    _components.add(k, std::move(v));
            }
        }

        template<typename T> bool hasInterface() const {
            if(const auto iter = _interface_map.find(Type<T>::id()); iter != _interface_map.end())
                return true;
            return _components.has<T>();
        }

        template<typename T> sp<T> getInterface() const {
            if(const auto iter = _interface_map.find(Type<T>::id()); iter != _interface_map.end())
                return iter->second.template toPtr<T>();
            if(sp<T> interfaceObj = _components.get<T>())
                return interfaceObj;
            return nullptr;
        }

        template<typename T> void setInterface(sp<T> interfaceObj) {
            DCHECK(!(_components.has<T>() || _interface_map.contains(Type<T>::id())), "Overriding interface: \"%s\"", Class::ensureClass<T>()->name());
            if(!_components.has<T>())
                _interface_map.insert_or_assign(Type<T>::id(), Box(std::move(interfaceObj)));
        }

        template<typename T> sp<T> ensureInterface() const {
            sp<T> interfaceObj = getInterface<T>();
            CHECK(interfaceObj, "Interface \"%s\" does not exist", Class::ensureClass<T>()->name());
            return interfaceObj;
        }

        template<typename T> bool hasComponent() const {
            if(const auto iter = _component_map.find(Type<T>::id()); iter != _component_map.end())
                return true;
            return _components.has<T>();
        }

        template<typename T> sp<T> getComponent() const {
            if(const auto iter = _component_map.find(Type<T>::id()); iter != _component_map.end())
                return iter->second.template toPtr<T>();
            if(sp<T> component = _components.get<T>())
                return component;
            return nullptr;
        }

        template<typename T> sp<T> ensureComponent() const {
            sp<T> component = getComponent<T>();
            CHECK(component, "Component \"%s\" does not exist", Class::ensureClass<T>()->name());
            return component;
        }

        template<typename T> void setComponent(sp<T> component) {
            CHECK_WARN(!(_upload_on_close && (_components.has<T>() || _component_map.contains(Type<T>::id()))), "Overriding component: \"%s\"", Class::ensureClass<T>()->name());
            if(!(_upload_on_close && _components.has<T>()))
                _component_map.insert_or_assign(Type<T>::id(), Box(std::move(component)));
        }

        template<typename T> T getEnum(T defaultValue) const {
            return _components.getEnum<T>(defaultValue);
        }

    private:
        Traits& _components;
        bool _upload_on_close;
        Map<TypeId, Box> _component_map;
        Map<TypeId, Box> _interface_map;
    };

    class Niche {
    public:
        virtual ~Niche() = default;

        virtual void onPoll(WiringContext& context, const document& component) = 0;
    };

    virtual ~Wirable() = default;

    virtual void onPoll(WiringContext& context) {}
    virtual void onWire(const WiringContext& context, const Box& self) = 0;

};

}
