#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/collection/traits.h"
#include "core/inf/debris.h"
#include "../impl/builder/safe_builder.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API ResourceLoader {
private:
    template<typename T> class BuilderRefs : public BoxBundle {
    public:
        BuilderRefs(const BeanFactory& beanFactory)
            : _bean_factory(beanFactory) {
        }

        Box get(const String& refid) override {
            const SafeBuilder<T> builder = getBuilder(Identifier::parseRef(refid));
            sp<T> ptr = builder.build({});
            DCHECK(ptr, "ResourceLoader has no object referred as \"%s\"", refid.c_str());
            return Box(std::move(ptr));
        }

        SafeBuilder<T> getBuilder(const Identifier& id) {
            DCHECK(id.isRef(), "Id \"%s\" must be a reference", (id.isArg() ? id.arg().c_str() : id.val().c_str()));

            if(id.package())
                return _bean_factory.createBuilderByRef<sp<T>>(id);

            const auto iter = _builders.find(id.ref());
            if(iter != _builders.end())
                return iter->second;
            SafeBuilder<T>& builder = _builders[id.ref()];
            builder = _bean_factory.createBuilderByRef<sp<T>>(id);
            return builder;
        }

    private:
        BeanFactory _bean_factory;
        HashMap<String, SafeBuilder<T>> _builders;

    };

    class PackageRefs : public BoxBundle {
    public:
        PackageRefs(const BeanFactory& beanFactory);

        virtual Box get(const String& name) override;

    private:
        BeanFactory _bean_factory;
        HashMap<String, sp<ResourceLoader>> _packages;
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
        return _builder_refs.ensure<BuilderRefs<T>>(_bean_factory)->getBuilder(id).build(args);
    }

//  [[script::bindings::property]]
    sp<BoxBundle> refs() const;
//  [[script::bindings::property]]
    sp<BoxBundle> packages() const;

    void import(const document& manifest, BeanFactory& beanFactory);

    const BeanFactory& beanFactory() const;
    BeanFactory& beanFactory();

//  [[plugin::builder]]
    class BUILDER : public Builder<ResourceLoader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ApplicationContext>& applicationContext);

        sp<ResourceLoader> build(const Scope& args) override;

    private:
        sp<ApplicationContext> _application_context;
        document _manifest;
        String _src;
    };

//  [[plugin::resource-loader::by-value]]
    class DICTIONARY : public Builder<ResourceLoader> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value, const sp<ApplicationContext>& applicationContext);

        sp<ResourceLoader> build(const Scope& args) override;

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
