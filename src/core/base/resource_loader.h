#pragma once

#include <unordered_map>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/collection/traits.h"
#include "core/inf/holder.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"
#include "core/util/holder_util.h"

namespace ark {

class ARK_API ResourceLoader {
private:
    template<typename T> class BuilderRefs : public BoxBundle {
    public:
        BuilderRefs(const BeanFactory& beanFactory)
            : _bean_factory(beanFactory) {
        }

        virtual Box get(const String& refid) override {
            SafePtr<Builder<T>> builder = getBuilder(Identifier::parseRef(refid));
            const sp<T> ptr = builder->build({});
            DCHECK(ptr, "ResourceLoader has no object referred as \"%s\"", refid.c_str());
            return ptr;
        }

        SafePtr<Builder<T>> getBuilder(const Identifier& id) {
            DCHECK(id.isRef(), "Id \"%s\" must be a reference", (id.isArg() ? id.arg().c_str() : id.val().c_str()));

            if(id.package())
                return _bean_factory.createBuilderByRef<T>(id);

            const auto iter = _builders.find(id.ref());
            if(iter != _builders.end())
                return iter->second;
            SafePtr<Builder<T>>& builder = _builders[id.ref()];
            builder = _bean_factory.createBuilderByRef<T>(id);
            return builder;
        }

    private:
        BeanFactory _bean_factory;
        std::unordered_map<String, SafePtr<Builder<T>>> _builders;

    };

    class PackageRefs : public BoxBundle {
    public:
        PackageRefs(const BeanFactory& beanFactory);

        virtual Box get(const String& name) override;

    private:
        BeanFactory _bean_factory;
        std::unordered_map<String, sp<ResourceLoader>> _packages;

    };

public:
    ResourceLoader(const BeanFactory& beanFactory);
    ~ResourceLoader();

//  [[script::bindings::loader]]
    template<typename T> sp<T> load(const String& name, const Scope& args) {
        DCHECK(name, "Empty name");
        if(name.at(0) == '#')
            return _bean_factory.getBuilder<T>(name.substr(1))->build(args);
        const Identifier id = name.at(0) == Identifier::ID_TYPE_REFERENCE ? Identifier::parse(name) : Identifier::parseRef(name);
        return _builder_refs.ensure<BuilderRefs<T>>(_bean_factory)->getBuilder(id)->build(args);
    }

//  [[script::bindings::property]]
    sp<BoxBundle> refs() const;
//  [[script::bindings::property]]
    sp<BoxBundle> layers();
//  [[script::bindings::property]]
    sp<BoxBundle> renderLayers();
//  [[script::bindings::property]]
    sp<BoxBundle> packages() const;

    void import(const document& manifest, BeanFactory& beanFactory);

    const BeanFactory& beanFactory() const;
    BeanFactory& beanFactory();

//  [[plugin::builder]]
    class BUILDER : public Builder<ResourceLoader> {
    public:
        BUILDER(BeanFactory& factory, const document& doc, const sp<ApplicationContext>& applicationContext);

        virtual sp<ResourceLoader> build(const Scope& args) override;

    private:
        BeanFactory _factory;
        sp<ApplicationContext> _application_context;
        document _manifest;
        String _src;
    };

//  [[plugin::resource-loader::by-value]]
    class DICTIONARY : public Builder<ResourceLoader> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value, const sp<ApplicationContext>& applicationContext);

        virtual sp<ResourceLoader> build(const Scope& args) override;

    private:
        String _src;
        sp<ApplicationContext> _application_context;
    };

private:
    BeanFactory _bean_factory;
    Traits _builder_refs;

    sp<PackageRefs> _packages;

};

}
