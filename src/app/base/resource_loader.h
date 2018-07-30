#ifndef ARK_APP_BASE_RESOURCE_LOADER_H_
#define ARK_APP_BASE_RESOURCE_LOADER_H_

#include <map>

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/collection/by_type.h"
#include "core/types/shared_ptr.h"
#include "core/forwarding.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"

namespace ark {

//[[script::bindings::container]]
class ARK_API ResourceLoader {
private:
    template<typename T> class BuilderCache {
    public:
        sp<Builder<T>> getBuilder(BeanFactory& factory, const Identifier& id) {
            DCHECK(id.isRef(), "Id \"%s\" must be a reference", (id.isArg() ? id.arg().c_str() : id.val().c_str()));
            const auto iter = _builders.find(id.ref());
            if(iter != _builders.end())
                return iter->second;
            const sp<Builder<T>> builder = factory.findBuilderById<T>(id, true);
            _builders[id.ref()] = builder;
            return builder;
        }

    private:
        std::map<String, sp<Builder<T>>> _builders;
    };

public:
    ResourceLoader(const BeanFactory& beanFactory);
    ~ResourceLoader();

//  [[script::bindings::loader]]
    template<typename T> sp<T> load(const String& name, const sp<Scope>& args = nullptr) {
        DCHECK(name, "Empty name");
        const Identifier id = name.at(0) == Identifier::ID_TYPE_REFERENCE ? Identifier::parse(name) : Identifier::parseRef(name);
        return _builder_caches.ensure<BuilderCache<T>>()->getBuilder(_bean_factory, id)->build(args);
    }

//  [[script::bindings::property]]
    const sp<Scope>& refs() const;

    void import(const document& manifest, BeanFactory& beanFactory);

    const BeanFactory& beanFactory() const;
    BeanFactory& beanFactory();

//  [[plugin::builder]]
    class BUILDER : public Builder<ResourceLoader> {
    public:
        BUILDER(BeanFactory& parent, const document& doc, const sp<ApplicationContext>& applicationContext);

        virtual sp<ResourceLoader> build(const sp<Scope>& args) override;

    private:
        BeanFactory _parent;
        sp<ApplicationContext> _application_context;
        document _manifest;
        String _src;
    };

//  [[plugin::resource-loader::by-value]]
    class DICTIONARY : public Builder<ResourceLoader> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value, const sp<ApplicationContext>& applicationContext);

        virtual sp<ResourceLoader> build(const sp<Scope>& args) override;

    private:
        String _src;
        sp<ApplicationContext> _application_context;
    };

private:
    BeanFactory _bean_factory;
    ByType _builder_caches;
};

}

#endif
