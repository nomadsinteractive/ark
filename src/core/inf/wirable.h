#pragma once

#include <map>

#include "core/forwarding.h"
#include "core/base/constants.h"
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
            _components.put(std::move(component));
        }

        template<typename T> void addComponentBuilder(sp<Builder<T>> componentBuilder) {
            _component_builders.insert_or_assign(Type<T>::id(), Box(std::move(componentBuilder)));
        }

    private:
        Traits& _components;
        std::map<TypeId, Box> _component_builders;
    };

    template<typename T, typename U> class BuilderWithRef final {
    public:
        BuilderWithRef(BeanFactory& factory, const document& manifest)
            : _delegate(makeRefBuilder(factory, manifest)) {
        }

        sp<Wirable> build(const Scope& args) const {
            return _delegate->build(args);
        }

    private:
        static sp<Builder<Wirable>> makeRefBuilder(BeanFactory& factory, const document& manifest) {
            sp<Builder<T>> delegate;
            if(const String ref = Documents::getAttribute(manifest, constants::REF))
                delegate = factory.ensureBuilder<T>(ref);
            else
                delegate = sp<U>::make(factory, manifest);
            return sp<Builder<Wirable>>::make<Builder<Wirable>::Wrapper<Builder<T>>>(std::move(delegate));
        }

    private:
        sp<Builder<Wirable>> _delegate;
    };

    virtual ~Wirable() = default;

    virtual TypeId onPoll(WiringContext& context) = 0;
    virtual void onWire(const WiringContext& context) = 0;

};

}
