#pragma once

#include <list>
#include <type_traits>

#include "core/base/api.h"
#include "core/base/constants.h"
#include "core/base/identifier.h"
#include "core/base/scope.h"
#include "core/collection/traits.h"
#include "core/dom/dom_document.h"
#include "core/inf/builder.h"
#include "core/inf/dictionary.h"
#include "core/util/documents.h"
#include "core/types/weak_ptr.h"

namespace ark {

class ARK_API BeanFactory {
private:
    template<typename T> class Worker {
    public:
        Worker() = default;

        sp<Builder<T>> createValueBuilder(BeanFactory& factory, const String& value) const {
            const sp<Builder<T>> builder = createValueBuilder(factory, value, value);
            return builder || !_default_dictionary_factory ? builder : _default_dictionary_factory(factory, value);
        }

        sp<Builder<T>> createValueBuilder(BeanFactory& factory, const String& type, const String& value) const {
            if(const auto iter = _values.find(type); iter != _values.end())
                return iter->second(factory, value);
            return nullptr;
        }

        void addBuilderFactory(const String& id, std::function<sp<Builder<T>>(BeanFactory&, const document&)> builderFactory) {
            _builders[id] = std::move(builderFactory);
        }

        void setDefaultBuilderFactory(std::function<sp<Builder<T>>(BeanFactory&, const document&)> builderFactory) {
            DCHECK(!_default_builder_factory, "Overriding existing Builder factory");
            _default_builder_factory = std::move(builderFactory);
        }

        void addDictionaryFactory(const String& value, std::function<sp<Builder<T>>(BeanFactory&, const String&)> dictionaryFactory) {
            _values[value] = std::move(dictionaryFactory);
        }

        void setDictionaryFactory(std::function<sp<Builder<T>>(BeanFactory&, const String&)> defaultDictionaryFactory) {
            DCHECK(!_default_dictionary_factory, "Overriding existing Dictionary factory");
            _default_dictionary_factory = std::move(defaultDictionaryFactory);
        }

        sp<Builder<T>> createBuilder(BeanFactory& factory, const document& doc, const bool wrapBuilder) const {
            const String className = Documents::getAttribute(doc, constants::CLASS);
            return createBuilder(factory, className, doc, wrapBuilder);
        }

        sp<Builder<T>> createBuilder(BeanFactory& factory, const String& className, const document& doc, bool wrapBuilder) const {
            const String& id = Documents::getAttribute(doc, constants::ID);
            const String& wrappedId = wrapBuilder ? id : "";
            if(const String& ref = Documents::getAttribute(doc, constants::REF); className.empty() && ref) {
                const Identifier f = Identifier::parse(ref);
                if(f.isRef()) {
                    if(f.package().empty() && (f.ref() == id || ref == id))
                        return createBuilderInternal(factory, className, ref, doc);

                    return makeWrappedBuilder<T>(factory.createBuilderByRef<T>(f), wrappedId, factory.references());
                }
                if(f.isArg())
                    return factory.getBuilderByArg<T>(f);
            }
            return createBuilderInternal(factory, className.empty() ? doc->name() : className, wrappedId, doc);
        }

    private:
        sp<Builder<T>> createBuilderInternal(BeanFactory& factory, const String& className, const String& id, const document& doc) const {
            const auto iter = _builders.find(className);
            if(iter == _builders.end())
                return _default_builder_factory ? makeWrappedBuilder<T>(_default_builder_factory(factory, doc), id, factory.references()) : nullptr;
            sp<Builder<T>> builder = iter->second(factory, doc);
            CHECK(builder, "Builder \"%s\" create failed", className.c_str());
            return makeWrappedBuilder<T>(std::move(builder), id, factory.references());
        }

    private:
        std::function<sp<Builder<T>>(BeanFactory&, const document&)> _default_builder_factory;
        std::function<sp<Builder<T>>(BeanFactory&, const String&)> _default_dictionary_factory;

        Map<String, std::function<sp<Builder<T>>(BeanFactory&, const String&)>> _values;
        Map<String, std::function<sp<Builder<T>>(BeanFactory&, const document&)>> _builders;

        friend class Factory;
        friend class BeanFactory;
    };

public:
    class ARK_API Factory {
    public:
        Factory() = default;
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Factory);

        explicit operator bool() const;

        template<typename T> sp<Builder<T>> createBuilder(const document& manifest, BeanFactory& factory) const {
            const sp<Worker<T>>& worker = _workers.get<Worker<T>>();
            return worker ? worker->createBuilder(factory, manifest, true) : nullptr;
        }

        template<typename T> sp<Builder<T>> createBuilder(const String& className, const document& doc, BeanFactory& factory, bool wrapBuilder) const {
            const sp<Worker<T>>& worker = _workers.get<Worker<T>>();
            return worker ? worker->createBuilder(factory, className, doc, wrapBuilder) : nullptr;
        }

        template<typename T> sp<Builder<T>> createValueBuilder(BeanFactory& factory, const String& value) const {
            const sp<Worker<T>>& worker = _workers.get<Worker<T>>();
            return worker ? worker->createValueBuilder(factory, value) : nullptr;
        }

        template<typename T> sp<Builder<T>> createValueBuilder(BeanFactory& factory, const String& type, const String& value) const {
            const sp<Worker<T>>& worker = _workers.get<Worker<T>>();
            return worker ? worker->createValueBuilder(factory, type, value) : nullptr;
        }

        template<typename T> void addDictionaryFactory(const String& name, std::function<sp<Builder<T>>(BeanFactory&, const String&)> dictionaryFactory) {
            ensureWorker<T>()->addDictionaryFactory(name, std::move(dictionaryFactory));
        }

        template<typename T> void addDictionaryFactory(std::function<sp<Builder<T>>(BeanFactory&, const String&)> dictionaryFactory) {
            ensureWorker<T>()->setDictionaryFactory(std::move(dictionaryFactory));
        }

        template<typename T> void addBuilderFactory(const String& id, std::function<sp<Builder<T>>(BeanFactory&, const document&)> builderFactory) {
            ensureWorker<T>()->addBuilderFactory(id, std::move(builderFactory));
        }

        template<typename T> void addBuilderFactory(std::function<sp<Builder<T>>(BeanFactory&, const document&)> builderFactory) {
            ensureWorker<T>()->setDefaultBuilderFactory(std::move(builderFactory));
        }

        template<typename T> sp<Worker<T>> getWorker() const {
            return _workers.get<Worker<T>>();
        }

        template<typename T> sp<Worker<T>> ensureWorker() {
            if(!_workers.has<Worker<T>>())
                _workers.put<Worker<T>>(sp<Worker<T>>::make());
            return _workers.get<Worker<T>>();
        }

    private:
        Traits _workers;
    };

    struct Stub {
        Stub(sp<Dictionary<document>> documentRefs);

        sp<Scope> _references;
        sp<Dictionary<document>> _document_refs;

        std::list<Factory> _factories;
        Map<String, sp<BeanFactory>> _packages;
    };

public:
    BeanFactory() = default;
    BeanFactory(sp<Dictionary<document>> documentRefs);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(BeanFactory);

    ~BeanFactory() = default;

    template<typename T> sp<Builder<T>> createBuilderByRef(const Identifier& id) {
        sp<Builder<T>> builder;
        if(id.package()) {
            sp<BeanFactory> factory = getPackage(id.package());
            CHECK(factory, "Id: \"%s\"'s package \"%s\" not found", id.toString().c_str(), id.package().c_str());
            builder = factory ? factory->getBuilderByRef<T>(id.withouPackage()) : nullptr;
            CHECK_WARN(builder, "Cannot build \"%s\" from package \"%s\"", id.toString().c_str(), id.package().c_str());
        }
        else
            builder = getBuilderByRef<T>(id);
        return builder ? builder : findBuilderByValue<T>(id.toString());
    }

    template<typename T> sp<T> build(const String& value, const Scope& args) {
        return buildSafe<T>(getBuilder<T>(value), args);
    }

    template<typename T> sp<T> build(const String& type, const String& value, const Scope& args) {
        return buildSafe<T>(findBuilderByTypeValue<T>(type, value), args);
    }

    template<typename T> sp<T> build(const document& doc, const Scope& args) {
        return buildSafe<T>(findBuilderByDocument<T>(doc, true), args);
    }

    template<typename T> sp<T> build(const document& doc, const String& attr, const Scope& args) {
        return buildSafe<T>(getBuilder<T>(doc, attr), args);
    }

    template<typename T> sp<T> ensure(const String& value, const Scope& args) {
        sp<T> obj = build<T>(value, args);
        CHECK(obj, "Could not build \"%s\"", value.c_str());
        return obj;
    }

    template<typename T> sp<T> ensureByTypeValue(const String& type, const String& value, const Scope& args) {
        return ensureBuilderByTypeValue<T>(type, value)->build(args);
    }

    template<typename T> sp<T> ensure(const document& doc, const Scope& args) {
        sp<T> obj = build<T>(doc, args);
        CHECK(obj, "Could not build \"%s\"", Documents::toString(doc).c_str());
        return obj;
    }

    template<typename T> sp<T> ensure(const document& doc, const String& attr, const Scope& args) {
        sp<T> obj = build<T>(doc, attr, args);
        CHECK(obj, "Could not build \"%s\" from \"%s\"", attr.c_str(), Documents::toString(doc).c_str());
        return obj;
    }

    template<typename T> sp<Builder<T>> getBuilder(const String& id, Identifier::Type idType = Identifier::ID_TYPE_AUTO) {
        if(id.empty())
            return nullptr;

        const Identifier f = Identifier::parse(id, idType);
        if constexpr (std::is_same_v<T, String>)
            return f.isArg() ? getBuilderByArg<T>(f) : findBuilderByValue<T>(id);
        if(f.isRef())
            return createBuilderByRef<T>(f);
        if(f.isArg())
            return getBuilderByArg<T>(f);

        if(f.type() == Identifier::ID_TYPE_VALUE_AND_TYPE)
            return findBuilderByTypeValue<T>(f.valType(), f.val());

        if(f.type() == Identifier::ID_TYPE_EXPRESSION)
            return findBuilderByValue<T>(f.val());

        return findBuilderByValue<T>(id);
    }

    template<typename T> sp<Builder<T>> getBuilder(const document& doc, const String& attr, const String& defValue = "") {
        const String attrValue = Documents::getAttribute(doc, attr, defValue);
        if(attrValue.empty()) {
            if(const document& child = doc->getChild(attr)) {
                const sp<Builder<T>> builder = findBuilderByDocument<T>(child, true);
                CHECK(builder, "Cannot build \"%s\" from \"%s\"", attr.c_str(), Documents::toString(doc).c_str());
                return builder;
            }
            return nullptr;
        }
        return ensureBuilder<T>(attrValue);
    }

    template<typename T> Vector<std::pair<String, builder<T>>> makeValueBuilderList() {
        Vector<std::pair<String, builder<T>>> list;
        for(const Factory& i : _stub->_factories)
            if(const sp<Worker<T>> worker = i.getWorker<T>())
                for(const auto& [k, v] : worker->_values)
                    list.emplace_back(k, v(*this, k));
        return list;
    }

    template<typename T> Vector<builder<T>> makeBuilderList(const document& doc, const String& nodeName) {
        Vector<builder<T>> list;
        if(const String attrValue = Documents::getAttribute(doc, nodeName))
            list.push_back(ensureBuilder<T>(attrValue));

        for(const document& i : doc->children(nodeName))
            list.push_back(ensureBuilder<T>(i));
        return list;
    }

    template<typename T, typename... Args> Vector<T> makeBuilderListObject(const document& doc, const String& nodeName, Args&&... args) {
        Vector<T> list;
        for(const document& i : nodeName ? doc->children(nodeName) : doc->children())
            list.emplace_back(*this, i, std::forward<Args>(args)...);
        return list;
    }

    template<typename T> sp<Builder<T>> getConcreteClassBuilder(const document& doc, const String& attr) {
        static_assert(!std::is_abstract_v<T>, "Not a concrete class");
        const String attrValue = Documents::getAttribute(doc, attr);
        if(attrValue.empty()) {
            const document& child = doc->getChild(attr);
            return findBuilderByDocument<T>(child ? child : doc, "", false);
        }
        return ensureBuilder<T>(attrValue);
    }

    template<typename T> sp<Builder<T>> getBuilderByArg(String argname) {
        return sp<BuilderByArgument<T>>::make(_stub->_references, std::move(argname));
    }

    template<typename T> sp<Builder<T>> getBuilderByArg(const Identifier& id) {
        CHECK(id.isArg(), "Cannot build \"%s\" because it's not an argument", id.toString().c_str());
        return sp<BuilderByArgument<T>>::make(_stub->_references, id.arg(), id.isOptional() ? sp<Builder<T>>::template make<typename Builder<T>::Null>() : findBuilderByValue<T>(id.toString()));
    }

    template<typename T> sp<Builder<T>> getBuilderByRef(const Identifier& id) {
        const String& refid = id.ref();
        if(sp<T> inst = _stub->_references->get(refid).template as<T>())
            return sp<typename Builder<T>::Prebuilt>::make(std::move(inst));

        if(const document manifest = _stub->_document_refs->get(refid))
            for(const Factory& i : _stub->_factories)
                if(const sp<Builder<T>> builder = i.createBuilder<T>(manifest, *this))
                    return builder;

        return nullptr;
    }

    template<typename T> sp<Builder<T>> ensureBuilder(const String& id, Identifier::Type idType = Identifier::ID_TYPE_AUTO) {
        DCHECK(id, "Empty value being built");
        sp<Builder<T>> builder = getBuilder<T>(id, idType);
        CHECK(builder, "Cannot find builder \"%s\"", id.c_str());
        return builder;
    }

    template<typename T> sp<Builder<T>> ensureBuilder(const document& doc) {
        sp<Builder<T>> builder = findBuilderByDocument<T>(doc, true);
        CHECK(builder, "Cannot not build from \"%s\"", Documents::toString(doc).c_str());
        return builder;
    }

    template<typename T> sp<Builder<T>> ensureBuilder(const document& doc, const String& attr) {
        sp<Builder<T>> builder = getBuilder<T>(doc, attr);
        CHECK(builder, "Cannot not build \"%s\" from \"%s\"", attr.c_str(), Documents::toString(doc).c_str());
        return builder;
    }

    template<typename T> sp<Builder<T>> ensureConcreteClassBuilder(const document& doc, const String& attr) {
        sp<Builder<T>> builder = getConcreteClassBuilder<T>(doc, attr);
        CHECK(builder, "Cannot not build \"%s\" from \"%s\"", attr.c_str(), Documents::toString(doc).c_str());
        return builder;
    }

    template<typename T> sp<Builder<T>> ensureBuilderByTypeValue(const String& type, const String& value) {
        sp<Builder<T>> builder = findBuilderByTypeValue<T>(type, value);
        CHECK(builder, "Cannot not build Type(%s) with value \"%s\"", type.c_str(), value.c_str());
        return builder;
    }

    void add(Factory factory, bool front = false);
    void addPackage(const String& name, const BeanFactory& package);

    void extend(const BeanFactory& other);
    sp<BeanFactory> getPackage(const String& name) const;

    const sp<Scope>& references() const;

    template<typename T> sp<Builder<T>> findBuilderByDocument(const document& doc, bool wrapBuilder) {
        const String className = Documents::getAttribute(doc, constants::CLASS);
        return findBuilderByDocument<T>(doc, className, wrapBuilder);
    }

    template<typename T> sp<Builder<T>> findBuilderByDocument(const document& doc, const String& className, bool wrapBuilder) {
        for(const Factory& i : _stub->_factories)
            if(sp<Builder<T>> builder = i.createBuilder<T>(className, doc, *this, wrapBuilder))
                return builder;
        return nullptr;
    }

    template<typename T> sp<Builder<T>> findBuilderByValue(const String& value) {
        for(const Factory& i : _stub->_factories)
            if(sp<Builder<T>> builder = i.createValueBuilder<T>(*this, value))
                return builder;
        return nullptr;
    }

    template<typename T> sp<Builder<T>> findBuilderByTypeValue(const String& type, const String& value) {
        for(const Factory& i : _stub->_factories)
            if(sp<Builder<T>> builder = i.createValueBuilder<T>(*this, type, value))
                return builder;
        return nullptr;
    }

private:
    template<typename T> static sp<T> buildSafe(const sp<Builder<T>>& builder, const Scope& args) {
        return builder ? builder->build(args) : nullptr;
    }

    template<typename T> static sp<Builder<T>> makeWrappedBuilder(sp<Builder<T>> builder, const String& id, const sp<Scope>& references) {
        if(id.empty() || id.at(0) != '@')
            return builder;
        return sp<Builder<T>>::template make<BuilderBySoftRef<T>>(id.substr(1), std::move(references), std::move(builder));
    }

    template<typename T> class BuilderByArgument final : public Builder<T> {
    public:
        BuilderByArgument(WeakPtr<Scope> references, String name, sp<Builder<T>> fallback = nullptr)
            : _references(std::move(references)), _name(std::move(name)), _fallback(std::move(fallback)) {
        }

        sp<T> build(const Scope& args) override {
            if(const Optional<Box> optVar = args.getObject(_name))
                if(sp<T> value = optVar->as<T>(); value || !optVar.value())
                    return value;

            if(const Optional<Box> optVar = _references.ensure()->getObject(_name))
                if(sp<T> value = optVar->as<T>(); value || !optVar.value())
                    return value;

            CHECK(_fallback, "Cannot get argument \"%s\"", _name.c_str());
            return _fallback ? _fallback->build(args) : nullptr;
        }

    private:
        WeakPtr<Scope> _references;
        String _name;
        sp<Builder<T>> _fallback;
    };

    template<typename T> class BuilderBySoftRef final : public Builder<T> {
    public:
        BuilderBySoftRef(String name, WeakPtr<Scope> references, sp<Builder<T>> delegate)
            : _name(std::move(name)), _references(std::move(references)), _delegate(std::move(delegate)) {
        }

        sp<T> build(const Scope& args) override {
            const sp<Scope> reference = _references.lock();
            CHECK(reference, "BeanFactory has been disposed");
            sp<T> inst = reference->get(_name).template as<T>();
            if(!inst) {
                inst = _delegate->build(args);
                CHECK(inst, "Cannot build \"%s\"", _name.c_str());
                reference->put(_name, Box(inst));
                _delegate = nullptr;
            }
            return inst;
        }

    private:
        String _name;
        WeakPtr<Scope> _references;
        sp<Builder<T>> _delegate;
    };

private:
    sp<Stub> _stub;
};

}
