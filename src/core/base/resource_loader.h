#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/collection/traits.h"
#include "core/inf/debris.h"
#include "core/impl/builder/safe_builder.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API ResourceLoader {
private:
    template<typename T> class BuilderRefs final : public BoxBundle {
    public:
        BuilderRefs(const BeanFactory& beanFactory)
            : _bean_factory(beanFactory) {
        }

        Box get(const String& refid) override {
            const SafeBuilder<T> builder = getBuilder(Identifier::parseRef(refid));
            sp<T> ptr = builder.build({});
            CHECK(ptr, "ResourceLoader has no object referred as \"%s\"", refid.c_str());
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
            CHECK(builder, "ResourceLoader can not create a builder for \"%s\"", id.ref().c_str());
            return builder;
        }

    private:
        BeanFactory _bean_factory;
        Map<String, SafeBuilder<T>> _builders;
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

    void import(const document& manifest, BeanFactory& beanFactory);

    const BeanFactory& beanFactory() const;
    BeanFactory& beanFactory();

//  [[plugin::builder]]
    class BUILDER final : public Builder<ResourceLoader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ApplicationContext>& applicationContext);

        sp<ResourceLoader> build(const Scope& args) override;

    private:
        sp<ApplicationContext> _application_context;
        document _manifest;
        String _src;
    };

//  [[plugin::resource-loader::by-value]]
    class DICTIONARY final : public Builder<ResourceLoader> {
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
};

}
