#pragma once

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
#include "core/util/strings.h"
#include "core/types/optional.h"
#include "core/types/weak_ptr.h"

namespace ark {

class ARK_API BeanFactory {
private:
    template<typename T> class Worker {
    public:
        Worker() = default;

        typedef sp<IBuilder<T>> BuilderType;

        BuilderType createValueBuilder(BeanFactory& factory, const String& value) const {
            BuilderType builder = createValueBuilder(factory, value, value);
            return builder || !_default_dictionary_factory ? builder : _default_dictionary_factory(factory, value);
        }

        BuilderType createValueBuilder(BeanFactory& factory, const String& type, const String& value) const {
            if(const auto iter = _values.find(type); iter != _values.end())
                return iter->second(factory, value);
            return nullptr;
        }

        void addBuilderFactory(const String& id, std::function<BuilderType(BeanFactory&, const document&)> builderFactory) {
            _builders[id] = std::move(builderFactory);
        }

        void setDefaultBuilderFactory(std::function<BuilderType(BeanFactory&, const document&)> builderFactory) {
            DCHECK(!_default_builder_factory, "Overriding existing Builder factory");
            _default_builder_factory = std::move(builderFactory);
        }

        void addDictionaryFactory(const String& value, std::function<BuilderType(BeanFactory&, const String&)> dictionaryFactory) {
            _values[value] = std::move(dictionaryFactory);
        }

        void setDictionaryFactory(std::function<BuilderType(BeanFactory&, const String&)> defaultDictionaryFactory) {
            DCHECK(!_default_dictionary_factory, "Overriding existing Dictionary factory");
            _default_dictionary_factory = std::move(defaultDictionaryFactory);
        }

        BuilderType createBuilder(BeanFactory& factory, const document& doc, const bool wrapBuilder) const {
            const String className = Documents::getAttribute(doc, constants::CLASS);
            return createBuilder(factory, className, doc, wrapBuilder);
        }

        BuilderType createBuilder(BeanFactory& factory, const String& className, const document& doc, const bool wrapBuilder) const {
            const String& id = Documents::getAttribute(doc, constants::ID);
            const String& wrappedId = wrapBuilder ? id : "";
            const String& ref = Documents::getAttribute(doc, constants::REF);
            CHECK(!(className && ref && className == Documents::getAttribute(doc, constants::CLASS)), "Both \"class\" and \"ref\" defined in \"%s\", typically it is a mistake.", Documents::toString(doc).c_str());
            if(className.empty() && ref) {
                const Identifier f = Identifier::parse(ref);
                if(f.isRef()) {
                    if(f.package().empty() && (f.ref() == id || ref == id))
                        return createBuilderInternal(factory, className, ref, doc);

                    return makeReferencedBuilder<T>(factory.createBuilderByRef<T>(f), wrappedId, factory.references());
                }
                if(f.isArg())
                    return factory.getBuilderByArg<T>(f);
            }
            return createBuilderInternal(factory, className.empty() ? doc->name() : className, wrappedId, doc);
        }

    private:
        BuilderType createBuilderInternal(BeanFactory& factory, const String& className, const String& id, const document& doc) const {
            const auto iter = _builders.find(className);
            if(iter == _builders.end())
                return _default_builder_factory ? makeReferencedBuilder<T>(_default_builder_factory(factory, doc), id, factory.references()) : nullptr;
            BuilderType builder = iter->second(factory, doc);
            CHECK(builder, "Builder \"%s\" create failed", className.c_str());
            return makeReferencedBuilder<T>(std::move(builder), id, factory.references());
        }

    private:
        std::function<BuilderType(BeanFactory&, const document&)> _default_builder_factory;
        std::function<BuilderType(BeanFactory&, const String&)> _default_dictionary_factory;

        Map<String, std::function<BuilderType(BeanFactory&, const String&)>> _values;
        Map<String, std::function<BuilderType(BeanFactory&, const document&)>> _builders;

        friend class Factory;
        friend class BeanFactory;
    };

public:
    class ARK_API Factory {
    public:
        Factory() = default;
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Factory);

        explicit operator bool() const;

        template<typename T> sp<IBuilder<T>> createBuilder(const document& manifest, BeanFactory& factory) const {
            const sp<Worker<T>>& worker = _workers.get<Worker<T>>();
            return worker ? worker->createBuilder(factory, manifest, true) : nullptr;
        }

        template<typename T> sp<IBuilder<T>> createBuilder(const String& className, const document& doc, BeanFactory& factory, bool wrapBuilder) const {
            const sp<Worker<T>>& worker = _workers.get<Worker<T>>();
            return worker ? worker->createBuilder(factory, className, doc, wrapBuilder) : nullptr;
        }

        template<typename T> sp<IBuilder<T>> createValueBuilder(BeanFactory& factory, const String& value) const {
            const sp<Worker<T>>& worker = _workers.get<Worker<T>>();
            return worker ? worker->createValueBuilder(factory, value) : nullptr;
        }

        template<typename T> sp<IBuilder<T>> createValueBuilder(BeanFactory& factory, const String& type, const String& value) const {
            const sp<Worker<T>>& worker = _workers.get<Worker<T>>();
            return worker ? worker->createValueBuilder(factory, type, value) : nullptr;
        }

        template<typename T> void addDictionaryFactory(const String& name, std::function<sp<IBuilder<T>>(BeanFactory&, const String&)> dictionaryFactory) {
            ensureWorker<T>()->addDictionaryFactory(name, std::move(dictionaryFactory));
        }

        template<typename T> void addDictionaryFactory(std::function<sp<IBuilder<T>>(BeanFactory&, const String&)> dictionaryFactory) {
            ensureWorker<T>()->setDictionaryFactory(std::move(dictionaryFactory));
        }

        template<typename T> void addBuilderFactory(const String& id, std::function<sp<IBuilder<T>>(BeanFactory&, const document&)> builderFactory) {
            ensureWorker<T>()->addBuilderFactory(id, std::move(builderFactory));
        }

        template<typename T> void addBuilderFactory(std::function<sp<IBuilder<T>>(BeanFactory&, const document&)> builderFactory) {
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

        List<Factory> _factories;
        Map<String, sp<BeanFactory>> _packages;
    };

public:
    BeanFactory() = default;
    BeanFactory(sp<Dictionary<document>> documentRefs);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(BeanFactory);

    ~BeanFactory() = default;

    template<typename T> sp<IBuilder<T>> createBuilderByRef(const Identifier& id) {
        sp<IBuilder<T>> builder;
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
        return buildSafe(getBuilder<T>(value), args);
    }

    template<typename T> sp<T> build(const String& type, const String& value, const Scope& args) {
        return buildSafe(findBuilderByTypeValue<sp<T>>(type, value), args);
    }

    template<typename T> sp<T> build(const document& doc, const Scope& args) {
        return buildSafe(findBuilderByDocument<sp<T>>(doc, true), args);
    }

    template<typename T> sp<T> build(const document& doc, const String& attr, const Scope& args) {
        return buildSafe(getBuilder<T>(doc, attr), args);
    }

    template<typename T> sp<T> ensure(const String& value, const Scope& args) {
        sp<T> obj = build<T>(value, args);
        CHECK(obj, "Could not build \"%s\"", value.c_str());
        return obj;
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

    template<typename T> sp<Builder<T>> getBuilder(const String& id, const Identifier::Type idType = Identifier::ID_TYPE_AUTO) {
        return getIBuilder<sp<T>>(id, idType);
    }
    template<typename T> sp<IBuilder<T>> getIBuilder(const String& id, const Identifier::Type idType = Identifier::ID_TYPE_AUTO) {
        if(id.empty())
            return nullptr;

        const Identifier f = Identifier::parse(id, idType);
        if constexpr (std::is_same_v<T, String> || std::is_same_v<T, sp<String>>)
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

    template<typename T> sp<IBuilder<T>> getIBuilder(const document& doc, const String& attr) {
        const String attrValue = Documents::getAttribute(doc, attr);
        if(attrValue.empty()) {
            if(const document& child = doc->getChild(attr)) {
                sp<IBuilder<T>> builder = findBuilderByDocument<T>(child, true);
                CHECK(builder, "Cannot build \"%s\" from \"%s\"", attr.c_str(), Documents::toString(doc).c_str());
                return builder;
            }
            return nullptr;
        }
        return ensureIBuilder<T>(attrValue);
    }
    template<typename T> sp<Builder<T>> getBuilder(const document& doc, const String& attr) {
        return getIBuilder<sp<T>>(doc, attr);
    }

    template<typename T> Vector<std::pair<String, builder<T>>> makeValueBuilderList() {
        Vector<std::pair<String, builder<T>>> list;
        for(const Factory& i : _stub->_factories)
            if(const sp<Worker<sp<T>>> worker = i.getWorker<sp<T>>())
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
            return findBuilderByDocument<sp<T>>(child ? child : doc, "", false);
        }
        return ensureBuilder<T>(attrValue);
    }

    template<typename T> sp<IBuilder<T>> getBuilderByArg(String argname) {
        return sp<IBuilder<T>>::template make<BuilderByArgument<T>>(_stub->_references, std::move(argname));
    }

    template<typename T> sp<IBuilder<T>> getBuilderByArg(const Identifier& id) {
        CHECK(id.isArg(), "Cannot build \"%s\" because it's not an argument", id.toString().c_str());
        return sp<IBuilder<T>>::template make<BuilderByArgument<T>>(_stub->_references, id.arg(), id.isOptional() ? sp<IBuilder<T>>::template make<typename IBuilder<T>::Null>() : findBuilderByValue<T>(id.toString()));
    }

    template<typename T> sp<IBuilder<T>> getBuilderByRef(const Identifier& id) {
        const String& refid = id.ref();
        if(sp<IBuilder<T>> r = getBuilderByRef_sfinae<T>(refid, nullptr))
            return r;

        if(const document manifest = _stub->_document_refs->get(refid))
            for(const Factory& i : _stub->_factories)
                if(sp<IBuilder<T>> builder = i.createBuilder<T>(manifest, *this))
                    return builder;

        return nullptr;
    }
    template<typename T> sp<IBuilder<T>> getBuilderByRef_sfinae(const String& refid, std::enable_if_t<std::is_same_v<sp<typename T::_PtrType>, T>>*) const {
        if(T inst = _stub->_references->get(refid).template as<typename T::_PtrType>())
            return sp<IBuilder<T>>::template make<typename IBuilder<T>::Prebuilt>(std::move(inst));
        return nullptr;
    }
    template<typename T> sp<IBuilder<T>> getBuilderByRef_sfinae(const String& /*refid*/, ...) const {
        return nullptr;
    }

    template<typename T> sp<IBuilder<T>> ensureIBuilder(const String& id, const Identifier::Type idType = Identifier::ID_TYPE_AUTO) {
        DCHECK(id, "Empty value being built");
        sp<IBuilder<T>> builder = getIBuilder<T>(id, idType);
        CHECK(builder, "Cannot find builder \"%s\"", id.c_str());
        return builder;
    }
    template<typename T> sp<Builder<T>> ensureBuilder(const String& id, const Identifier::Type idType = Identifier::ID_TYPE_AUTO) {
        return ensureIBuilder<sp<T>>(id, idType);
    }

    template<typename T> sp<Builder<T>> ensureBuilder(const document& manifest) {
        sp<Builder<T>> builder = findBuilderByDocument<sp<T>>(manifest, true);
        CHECK(builder, "Cannot not build from \"%s\"", Documents::toString(manifest).c_str());
        return builder;
    }

    template<typename T> sp<Builder<T>> ensureBuilder(const document& manifest, const String& attr) {
        sp<Builder<T>> builder = getBuilder<T>(manifest, attr);
        CHECK(builder, "Cannot not build \"%s\" from \"%s\"", attr.c_str(), Documents::toString(manifest).c_str());
        return builder;
    }

    template<typename T> sp<Builder<T>> ensureConcreteClassBuilder(const document& doc, const String& attr) {
        sp<Builder<T>> builder = getConcreteClassBuilder<T>(doc, attr);
        CHECK(builder, "Cannot not build \"%s\" from \"%s\"", attr.c_str(), Documents::toString(doc).c_str());
        return builder;
    }

    template<typename T> sp<IBuilder<T>> ensureBuilderByTypeValue(const String& type, const String& value) {
        sp<IBuilder<T>> builder = findBuilderByTypeValue<T>(type, value);
        CHECK(builder, "Cannot not build Type(%s) with value \"%s\"", type.c_str(), value.c_str());
        return builder;
    }

    void add(Factory factory, bool front = false);
    void addPackage(const String& name, const BeanFactory& package);

    void extend(const BeanFactory& other);
    sp<BeanFactory> getPackage(const String& name) const;

    const sp<Scope>& references() const;

    template<typename T> sp<IBuilder<T>> findBuilderByDocument(const document& doc, const String& className, const bool wrapBuilder) {
        for(const Factory& i : _stub->_factories)
            if(sp<IBuilder<T>> builder = i.createBuilder<T>(className, doc, *this, wrapBuilder))
                return builder;
        return nullptr;
    }

    template<typename T> sp<IBuilder<T>> findBuilderByTypeValue(const String& type, const String& value) {
        for(const Factory& i : _stub->_factories)
            if(sp<IBuilder<T>> builder = i.createValueBuilder<T>(*this, type, value))
                return builder;
        return nullptr;
    }

    template<typename... Args> void expand(const String& expr, sp<IBuilder<Args>>&... args) {
        const String value = Strings::unwrap(expr.strip(), '(', ')');
        CHECK(value, "Empty value being built");
        auto elements = Strings::split<List<String>>(value, ',');
        doExpand<Args...>(elements, args...);
    }

private:
    template<typename T, typename... Args> void doExpand(List<String>& elems, sp<IBuilder<T>>& builder, sp<IBuilder<Args>>&... args) {
        if(elems.empty())
            builder = sp<IBuilder<T>>::template make<typename IBuilder<T>::Null>();
        else {
            const String str = elems.front().strip();
            elems.pop_front();
            builder = getIBuilder<T>(str);
        }
        if constexpr(sizeof...(args) > 0)
            doExpand<Args...>(elems, args...);
    }

    template<typename T> static T buildSafe(const sp<IBuilder<T>>& builder, const Scope& args) {
        return builder ? builder->build(args) : T();
    }

    template<typename T> sp<IBuilder<T>> findBuilderByDocument(const document& doc, const bool wrapBuilder) {
        const String className = Documents::getAttribute(doc, constants::CLASS);
        return findBuilderByDocument<T>(doc, className, wrapBuilder);
    }

    template<typename T> sp<IBuilder<T>> findBuilderByValue(const String& value) {
        for(const Factory& i : _stub->_factories)
            if(sp<IBuilder<T>> builder = i.createValueBuilder<T>(*this, value))
                return builder;
        return nullptr;
    }

    template<typename T> static sp<IBuilder<T>> makeReferencedBuilder(sp<IBuilder<T>> builder, const String& id, const sp<Scope>& references) {
        if(id.empty() || id.at(0) != '@')
            return builder;
        return sp<IBuilder<T>>::template make<BuilderBySoftRef<T>>(id.substr(1), std::move(references), std::move(builder));
    }

    template<typename T> class BuilderByArgument final : public IBuilder<T> {
    public:
        BuilderByArgument(WeakPtr<Scope> references, String name, sp<IBuilder<T>> fallback = nullptr)
            : _references(std::move(references)), _name(std::move(name)), _fallback(std::move(fallback)) {
        }

        T build(const Scope& args) override {
            if(Optional<T> opt = args.getObject<T>(_name))
                return std::move(opt.value());

            if(Optional<T> opt = _references.ensure()->template getObject<T>(_name))
                return std::move(opt.value());

            CHECK(_fallback, "Cannot get argument \"%s\"", _name.c_str());
            return _fallback ? _fallback->build(args) : T();
        }

    private:
        WeakPtr<Scope> _references;
        String _name;
        sp<IBuilder<T>> _fallback;
    };

    template<typename T> class BuilderBySoftRef final : public IBuilder<T> {
    public:
        BuilderBySoftRef(String name, WeakPtr<Scope> references, sp<IBuilder<T>> delegate)
            : _name(std::move(name)), _references(std::move(references)), _delegate(std::move(delegate)) {
        }

        T build(const Scope& args) override {
            const sp<Scope> reference = _references.lock();
            CHECK(reference, "BeanFactory has been disposed");
            if(Optional<T> opt = reference->getObject<T>(_name))
                return std::move(opt.value());

            T inst = _delegate->build(args);
            reference->putObject<T>(_name, inst);
            return inst;
        }

    private:
        String _name;
        WeakPtr<Scope> _references;
        sp<IBuilder<T>> _delegate;
    };

private:
    sp<Stub> _stub;
};

}
