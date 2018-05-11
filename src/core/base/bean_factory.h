#ifndef ARK_CORE_BEAN_FACTORY_H_
#define ARK_CORE_BEAN_FACTORY_H_

#include <map>
#include <type_traits>

#include "core/base/api.h"
#include "core/collection/by_type.h"
#include "core/dom/document.h"
#include "core/inf/builder.h"
#include "core/inf/dictionary.h"
#include "core/impl/builder/builder_by_arguments.h"
#include "core/impl/builder/builder_by_hard_ref.h"
#include "core/impl/builder/builder_by_instance.h"
#include "core/impl/builder/builder_by_soft_ref.h"
#include "core/base/scope.h"
#include "core/types/box.h"
#include "core/util/identifier.h"
#include "core/util/documents.h"

namespace ark {

class ARK_API BeanFactory {
private:
    template<typename T> class NullBuilder : public Builder<T> {
    public:
        virtual sp<T> build(const sp<Scope>& /*args*/) override {
            return nullptr;
        }
    };

    template<typename T> class Worker {
    public:
        Worker(const WeakPtr<Scope>& references, const sp<Dictionary<document>>& documentById)
            : _references(references), _document_by_id(documentById) {
        }

        sp<Builder<T>> findBuilder(const String& id, BeanFactory& factory) {
            const document doc = _document_by_id->get(id);
            if(!doc)
                return nullptr;
            const sp<Builder<T>> builder = createBuilder(factory, doc);
            return builder;
        }

        sp<Builder<T>> createValueBuilder(BeanFactory& factory, const String& value) const {
            const sp<Builder<T>> builder = createValueBuilder(factory, value, value);
            return builder || !_default_dictionary_factory ? builder : _default_dictionary_factory(factory, value);
        }

        sp<Builder<T>> createValueBuilder(BeanFactory& factory, const String& type, const String& value) const {
            auto iter = _dictionary_factories.find(type);
            if(iter != _dictionary_factories.end())
                return iter->second(factory, value);
            return nullptr;
        }

        sp<Builder<T>> decorate(BeanFactory& factory, const sp<Builder<T>>& builder, const String& style, const String& value) const {
            auto iter = _builder_decorators.find(style);
            return iter != _builder_decorators.end() ? iter->second(factory, builder, value) : nullptr;
        }

        void addBuilderFactory(const String& id, std::function<sp<Builder<T>>(BeanFactory&, const document&)> builderFactory) {
            _builder_factories[id] = builderFactory;
        }

        void addBuilderDecorator(const String& style, std::function<sp<Builder<T>>(BeanFactory&, const sp<Builder<T>>&, const String&)> builderDecorator) {
            _builder_decorators[style] = builderDecorator;
        }

        void setDefaultBuilderFactory(std::function<sp<Builder<T>>(BeanFactory&, const document&)>&& builderFactory) {
            DCHECK(!_default_builder_factory, "Overriding existing Builder factory");
            _default_builder_factory = std::move(builderFactory);
        }

        void addDictionaryFactory(const String& value, std::function<sp<Builder<T>>(BeanFactory&, const String&)> dictionaryFactory) {
            _dictionary_factories[value] = dictionaryFactory;
        }

        void setDictionaryFactory(std::function<sp<Builder<T>>(BeanFactory&, const String&)> defaultDictionaryFactory) {
            DCHECK(!_default_dictionary_factory, "Overriding existing Dictionary factory");
            _default_dictionary_factory = defaultDictionaryFactory;
        }

        sp<Builder<T>> createBuilder(BeanFactory& factory, const document& doc) const {
            const String className = Documents::getAttribute(doc, Constants::Attributes::CLASS);
            return createBuilder(factory, className, doc);
        }

        sp<Builder<T>> createBuilder(BeanFactory& factory, const String& className, const document& doc) const {
            const String id = Documents::getAttribute(doc, Constants::Attributes::ID);
            const String ref = Documents::getAttribute(doc, Constants::Attributes::REF);
            DCHECK(ref.empty() || ref.at(0) != '#', "Reference \"%s\" may not be hard reference", ref.c_str());
            const String style = Documents::getAttribute(doc, Constants::Attributes::STYLE);
            if(className.empty()) {
                do {
                    if(ref) {
                        const Identifier f = Identifier::parse(ref);
                        if(f.isRef()) {
                            if(f.package().empty() && (f.ref() == id || ref == id))
                                break;
                            const sp<Builder<T>> builder = factory.findBuilderById<T>(f, true);
                            return decorateBuilder(factory, builder, style);
                        }
                        if(f.isArg())
                            return decorateBuilder(factory, sp<BuilderByArguments<T>>::make(f.arg(), _references), style);
                    }
                    return createBuilderInternal(factory, doc->name(), style, id, doc);
                } while(false);
            }
            DWARN(ref.empty() || ref.at(0) == '@', "Building class \"%s\" with reference \"%s\" has no effect", className.c_str(), ref.c_str());
            return createBuilderInternal(factory, className, style, ref ? ref : id, doc);
        }

    private:
        sp<Builder<T>> createBuilderInternal(BeanFactory& factory, const String& className, const String& style, const String& id, const document& doc) const {
            const auto iter = _builder_factories.find(className);
            if(iter == _builder_factories.end())
                return _default_builder_factory ? wrapBuilder(decorateBuilder(factory, _default_builder_factory(factory, doc), style), id) : nullptr;
            const sp<Builder<T>> builder = iter->second(factory, doc);
            DCHECK(builder, "Builder \"%s\" create failed", className.c_str());
            return wrapBuilder(decorateBuilder(factory, builder, style), id);
        }

        sp<Builder<T>> wrapBuilder(const sp<Builder<T>>& builder, const String& id) const {
            if(id.empty())
                return builder;
            if(id.at(0) == '@')
                return sp<Builder<T>>::adopt(new BuilderBySoftRef<T>(id.substr(1), _references, builder));
            if(id.at(0) == '#')
                return sp<Builder<T>>::adopt(new BuilderByHardRef<T>(id.substr(1), _references, builder));
            return builder;
        }

        sp<Builder<T>> decorateBuilder(BeanFactory& factory, const sp<Builder<T>>& builder, const String& styles) const {
            if(styles) {
                sp<Builder<T>> f = builder;
                for(const String& style : styles.split(';')) {
                    if(style.at(0) == '@') {
                        const document node = _document_by_id->get(style.substr(1));
                        DCHECK(node, "Style \"%s\" not found", style.substr(1).c_str());
                        f = factory.decorate<T>(f, node);
                    } else {
                        String key, value;
                        if(Strings::parseNameValuePair(style, ':', key, value))
                            f = factory.decorate<T>(f, key, value);
                    }
                }
                return f;
            }
            return builder;
        }

    private:
        WeakPtr<Scope> _references;
        sp<Dictionary<document>> _document_by_id;

        std::function<sp<Builder<T>>(BeanFactory&, const document&)> _default_builder_factory;
        std::function<sp<Builder<T>>(BeanFactory&, const String&)> _default_dictionary_factory;

        std::map<String, std::function<sp<Builder<T>>(BeanFactory&, const String&)>> _dictionary_factories;
        std::map<String, std::function<sp<Builder<T>>(BeanFactory&, const document&)>> _builder_factories;
        std::map<String, std::function<sp<Builder<T>>(BeanFactory&, const sp<Builder<T>>&, const String&)>> _builder_decorators;
    };

public:
    class ARK_API Factory {
    public:
        Factory();
        Factory(const WeakPtr<Scope>& references, const sp<Dictionary<document>>& documentById);

        explicit operator bool() const;

        template<typename T> sp<Builder<T>> findBuilder(const String& id, BeanFactory& factory) const {
            const sp<Worker<T>>& worker = _workers.get<Worker<T>>();
            return worker ? worker->findBuilder(id, factory) : nullptr;
        }

        template<typename T> sp<Builder<T>> createBuilder(const String& className, const document& doc, BeanFactory& factory) const {
            const sp<Worker<T>>& worker = _workers.get<Worker<T>>();
            return worker ? worker->createBuilder(factory, className, doc) : nullptr;
        }

        template<typename T> sp<Builder<T>> createValueBuilder(BeanFactory& factory, const String& value) const {
            const sp<Worker<T>>& worker = _workers.get<Worker<T>>();
            return worker ? worker->createValueBuilder(factory, value) : nullptr;
        }

        template<typename T> sp<Builder<T>> createValueBuilder(BeanFactory& factory, const String& type, const String& value) const {
            const sp<Worker<T>>& worker = _workers.get<Worker<T>>();
            return worker ? worker->createValueBuilder(factory, type, value) : nullptr;
        }

        template<typename T> sp<Builder<T>> decorate(BeanFactory& factory, const sp<Builder<T>>& builder, const String& style, const String& value) const {
            const sp<Worker<T>>& worker = _workers.get<Worker<T>>();
            return worker ? worker->decorate(factory, builder, style, value) : nullptr;
        }

        template<typename T> void addDictionaryFactory(const String& name, std::function<sp<Builder<T>>(BeanFactory&, const String&)> dictionaryFactory) {
            ensureWorker<T>()->addDictionaryFactory(name, dictionaryFactory);
        }

        template<typename T> void addDictionaryFactory(std::function<sp<Builder<T>>(BeanFactory&, const String&)> dictionaryFactory) {
            ensureWorker<T>()->setDictionaryFactory(dictionaryFactory);
        }

        template<typename T> void addBuilderFactory(const String& id, std::function<sp<Builder<T>>(BeanFactory&, const document&)> builderFactory) {
            ensureWorker<T>()->addBuilderFactory(id, builderFactory);
        }

        template<typename T> void addBuilderFactory(std::function<sp<Builder<T>>(BeanFactory&, const document&)>&& builderFactory) {
            ensureWorker<T>()->setDefaultBuilderFactory(std::move(builderFactory));
        }

        template<typename T> void addBuilderDecorator(const String& style, std::function<sp<Builder<T>>(BeanFactory&, const sp<Builder<T>>&, const String&)> builderDecorator) {
            ensureWorker<T>()->addBuilderDecorator(style, builderDecorator);
        }

    private:
        template<typename T> const sp<Worker<T>>& ensureWorker() {
            if(!_workers.has<Worker<T>>())
                _workers.put<Worker<T>>(sp<Worker<T>>::make(_references, _document_by_id));
            return _workers.get<Worker<T>>();
        }

    private:
        WeakPtr<Scope> _references;
        sp<Dictionary<document>> _document_by_id;

        ByType _workers;
    };

public:
    BeanFactory(std::nullptr_t);
    BeanFactory();
    BeanFactory(const BeanFactory& other) = default;
    BeanFactory(BeanFactory&& other) = default;
    ~BeanFactory();

    template<typename T> sp<T> load(const String& id, const sp<Scope>& args = nullptr) {
        const Identifier f = Identifier::parseRef(id);
        const sp<Builder<T>> builder = findBuilderById<T>(f, true);
        return builder->build(args);
    }

    template<typename T> sp<Builder<T>> findBuilderById(const Identifier& id, bool noNull) {
        if(id.package()) {
            const sp<BeanFactory>& factory = getPackage(id.package());
            DCHECK(noNull || factory, "BeanFactory \"%s\" not found");
            return factory ? factory->findBuilder<T>(id.ref(), noNull) : (noNull ? getNullBuilder<T>() : nullptr);
        }
        return findBuilder<T>(id.ref(), noNull);
    }

    template<typename T> sp<T> build(const String& value, const sp<Scope>& args = nullptr) {
        return getBuilder<T>(value, true)->build(args);
    }

    template<typename T> sp<T> build(const String& type, const String& value, const sp<Scope>& args = nullptr) {
        return createBuilderByTypeValue<T>(type, value, true)->build(args);
    }

    template<typename T> sp<T> build(const document& doc, const sp<Scope>& args = nullptr) {
        return createBuilderByDocument<T>(doc, true)->build(args);
    }

    template<typename T> sp<T> build(const String& className, const document& doc, const sp<Scope>& args = nullptr) {
        return createBuilderByDocument<T>(className, doc, true)->build(args);
    }

    template<typename T> sp<T> build(const document& doc, const String& attr, const sp<Scope>& args = nullptr) {
        return getBuilder<T>(doc, attr, true)->build(args);
    }

    template<typename T> sp<T> ensure(const String& value, const sp<Scope>& args = nullptr) {
        const sp<T> obj = build<T>(value, args);
        DCHECK(obj, "Counld not build \"%s\"", value.c_str());
        return obj;
    }

    template<typename T> sp<T> ensure(const String& type, const String& value, const sp<Scope>& args = nullptr) {
        return ensureBuilderByTypeValue<T>(type, value)->build(args);
    }

    template<typename T> sp<T> ensure(const document& doc, const sp<Scope>& args = nullptr) {
        const sp<T> obj = build<T>(doc, args);
        DCHECK(obj, "Counld not build \"%s\"", Documents::toString(doc).c_str());
        return obj;
    }

    template<typename T> sp<T> ensure(const document& doc, const String& attr, const sp<Scope>& args = nullptr) {
        const sp<T> obj = build<T>(doc, attr, args);
        DCHECK(obj, "Counld not build \"%s\" from \"%s\"", attr.c_str(), Documents::toString(doc).c_str());
        return obj;
    }

    template<typename T> sp<Builder<T>> getBuilder(const String& id, bool noNull = true) {
        if(id.empty())
            return noNull ? getNullBuilder<T>() : nullptr;

        const Identifier f = Identifier::parse(id);
        if(f.isRef())
            return findBuilderById<T>(f, noNull);
        if(f.isArg())
            return sp<BuilderByArguments<T>>::make(f.arg(), _references);
        return createBuilderByValue<T>(id, noNull);
    }

    template<typename T> sp<Builder<T>> getBuilder(const document& doc, const String& attr, bool noNull = true) {
        const String attrValue = Documents::getAttribute(doc, attr);
        if(attrValue.empty()) {
            const document& child = doc->getChild(attr);
            return child ? createBuilderByDocument<T>(child, noNull) : (noNull ? getNullBuilder<T>() : nullptr);
        }
        return getBuilder<T>(attrValue, noNull);
    }

    template<typename T> sp<Builder<T>> getConcreteClassBuilder(const document& doc, const String& attr, bool noNull = true) {
        static_assert(!std::is_abstract<T>::value, "Not a concrete class");
        const String attrValue = Documents::getAttribute(doc, attr);
        if(attrValue.empty()) {
            const document& child = doc->getChild(attr);
            return createBuilderByDocument<T>(child ? child : doc, noNull);
        }
        return getBuilder<T>(attrValue, noNull);
    }

    template<typename T> sp<Builder<T>> ensureBuilder(const String& id) {
        DCHECK(!id.empty(), "Empty value being built");
        const sp<Builder<T>> builder = getBuilder<T>(id, false);
        DCHECK(builder, "Could find builder \"%s\"", id.c_str());
        return builder;
    }

    template<typename T> sp<Builder<T>> ensureBuilder(const document& doc) {
        const sp<Builder<T>> builder = createBuilderByDocument<T>(doc, false);
        DCHECK(builder, "Counld not build from \"%s\"", Documents::toString(doc).c_str());
        return builder;
    }

    template<typename T> sp<Builder<T>> ensureBuilderByClass(const String& className, const document& doc) {
        const sp<Builder<T>> builder = createBuilderByDocument<T>(className, doc, false);
        DCHECK(builder, "Counld not build class \"%s\" from \"%s\"", className.c_str(), Documents::toString(doc).c_str());
        return builder;
    }

    template<typename T> sp<Builder<T>> ensureBuilder(const document& doc, const String& attr) {
        const sp<Builder<T>> builder = getBuilder<T>(doc, attr, false);
        DCHECK(builder, "Counld not build \"%s\" from \"%s\"", attr.c_str(), Documents::toString(doc).c_str());
        return builder;
    }

    template<typename T> sp<Builder<T>> ensureBuilderByTypeValue(const String& type, const String& value) {
        const sp<Builder<T>> builder = createBuilderByTypeValue<T>(type, value, false);
        DCHECK(builder, "Counld not build \"%s\" with value \"%s\"", type.c_str(), value.c_str());
        return builder;
    }

    template<typename T> sp<Builder<T>> decorate(const sp<Builder<T>>& builder, const String& properties) {
        const document style = Documents::fromProperties(properties);
        return decorate<T>(builder, style);
    }

    template<typename T> sp<Builder<T>> decorate(const sp<Builder<T>>& builder, const document& style) {
        sp<Builder<T>> f = builder;
        for(const attribute& attr : style->attributes())
            f = decorate<T>(f, attr->name(), attr->value());
        return f;
    }

    template<typename T> void setReference(const String& name, const sp<T>& reference) const {
        _references->put<T>(name, reference);
    }

    template<typename T> sp<T> getReference(const String& name) const {
        return _references->get<T>(name);
    }

    void add(const Factory& factory, bool front = false);
    void addPackage(const String& name, const BeanFactory& package);

    void extend(const BeanFactory& other);

    const sp<Scope>& references() const;

private:
    template<typename T> sp<Builder<T>> createBuilderByDocument(const document& doc, bool noNull) {
        const String className = Documents::getAttribute(doc, Constants::Attributes::CLASS);
        return createBuilderByDocument<T>(className, doc, noNull);
    }

    template<typename T> sp<Builder<T>> createBuilderByDocument(const String& className, const document& doc, bool noNull) {
        for(Factory& i : _factories->items()) {
            const sp<Builder<T>> builder = i.createBuilder<T>(className, doc, *this);
            if(builder)
                return builder;
        }
        return noNull ? getNullBuilder<T>() : nullptr;
    }

    template<typename T> sp<Builder<T>> createBuilderByValue(const String& value, bool noNull) {
        for(Factory& i : _factories->items()) {
            const sp<Builder<T>> builder = i.createValueBuilder<T>(*this, value);
            if(builder)
                return builder;
        }
        return noNull ? getNullBuilder<T>() : nullptr;
    }

    template<typename T> sp<Builder<T>> createBuilderByTypeValue(const String& type, const String& value, bool noNull) {
        for(Factory& i : _factories->items()) {
            const sp<Builder<T>> builder = i.createValueBuilder<T>(*this, type, value);
            if(builder)
                return builder;
        }
        return noNull ? getNullBuilder<T>() : nullptr;
    }

    template<typename T> sp<Builder<T>> findBuilder(const String& id, bool noNull) {
        const sp<T>& inst = getReference<T>(id);
        if(inst)
            return sp<BuilderByInstance<T>>::make(inst);

        for(const Factory& i : _factories->items()) {
            const sp<Builder<T>> builder = i.findBuilder<T>(id, *this);
            if(builder)
                return builder;
        }
        return noNull ? getNullBuilder<T>() : nullptr;
    }

    template<typename T> sp<Builder<T>> decorate(const sp<Builder<T>>& builder, const String& style, const String& value) {
        for(Factory& i : _factories->items()) {
            const sp<Builder<T>> f = i.decorate<T>(*this, builder, style, value);
            if(f)
                return f;
        }
        return builder;
    }

    const sp<BeanFactory>& getPackage(const String& name) const {
        DCHECK(name, "Empty package name");
        auto iter = _packages.find(name);
        return iter == _packages.end() ? sp<BeanFactory>::null() : iter->second;
    }

    template<typename T> sp<Builder<T>> getNullBuilder() const {
        static const sp<Builder<T>> nb = sp<NullBuilder<T>>::make();
        return nb;
    }

private:
    sp<Scope> _references;
    sp<List<Factory>> _factories;
    std::map<String, sp<BeanFactory>> _packages;
};

}

#endif
