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
        Worker(const WeakPtr<Scope>& references, const sp<Dictionary<document>>& documentById)
            : _references(references), _document_by_id(documentById) {
        }

        sp<Builder<T>> findBuilder(const String& id, BeanFactory& factory) {
            const document manifest = _document_by_id->get(id);
            if(!manifest)
                return nullptr;
            return createBuilder(factory, manifest, true);
        }

        sp<Builder<T>> createValueBuilder(BeanFactory& factory, const String& value) const {
            const sp<Builder<T>> builder = createValueBuilder(factory, value, value);
            return builder || !_default_dictionary_factory ? builder : _default_dictionary_factory(factory, value);
        }

        sp<Builder<T>> createValueBuilder(BeanFactory& factory, const String& type, const String& value) const {
            auto iter = _values.find(type);
            if(iter != _values.end())
                return iter->second(factory, value);
            return nullptr;
        }

        sp<Builder<T>> decorate(BeanFactory& factory, const sp<Builder<T>>& builder, const String& style, const String& value) const {
            auto iter = _decorators.find(style);
            return iter != _decorators.end() ? iter->second(factory, builder, value) : nullptr;
        }

        void addBuilderFactory(const String& id, std::function<sp<Builder<T>>(BeanFactory&, const document&)> builderFactory) {
            _builders[id] = std::move(builderFactory);
        }

        void addBuilderDecorator(const String& style, std::function<sp<Builder<T>>(BeanFactory&, const sp<Builder<T>>&, const String&)> builderDecorator) {
            _decorators[style] = std::move(builderDecorator);
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

        sp<Builder<T>> createBuilder(BeanFactory& factory, const document& doc, bool wrapBuilder) const {
            const String className = Documents::getAttribute(doc, constants::CLASS);
            return createBuilder(factory, className, doc, wrapBuilder);
        }

        sp<Builder<T>> createBuilder(BeanFactory& factory, const String& className, const document& doc, bool wrapBuilder) const {
            const String id = Documents::getAttribute(doc, constants::ID);
            const String wrappedId = wrapBuilder ? id : "";
            const String ref = Documents::getAttribute(doc, constants::REF);
            const String style = Documents::getAttribute(doc, constants::STYLE);
            if(className.empty() && ref) {
                const Identifier f = Identifier::parse(ref);
                if(f.isRef()) {
                    if(f.package().empty() && (f.ref() == id || ref == id))
                        return createBuilderInternal(factory, className, style, ref, doc);
                    const sp<Builder<T>> builder = factory.createBuilderByRef<T>(f);
                    return makeWrappedBuilder(makeDecoratedBuilder(factory, builder, style), wrappedId);
                }
                if(f.isArg())
                    return makeDecoratedBuilder(factory, factory.getBuilderByArg<T>(f), style);
            }
            return createBuilderInternal(factory, className.empty() ? doc->name() : className, style, wrappedId, doc);
        }

    private:
        sp<Builder<T>> createBuilderInternal(BeanFactory& factory, const String& className, const String& style, const String& id, const document& doc) const {
            const auto iter = _builders.find(className);
            if(iter == _builders.end())
                return _default_builder_factory ? makeWrappedBuilder(makeDecoratedBuilder(factory, _default_builder_factory(factory, doc), style), id) : nullptr;
            const sp<Builder<T>> builder = iter->second(factory, doc);
            CHECK(builder, "Builder \"%s\" create failed", className.c_str());
            return makeWrappedBuilder(makeDecoratedBuilder(factory, builder, style), id);
        }

        sp<Builder<T>> makeWrappedBuilder(sp<Builder<T>> builder, const String& id) const;

        sp<Builder<T>> makeDecoratedBuilder(BeanFactory& factory, const sp<Builder<T>>& builder, const String& styles) const {
            if(styles && builder) {
                sp<Builder<T>> f = builder;
                for(const String& style : styles.split(';')) {
                    if(style.at(0) == '@') {
                        const document node = _document_by_id->get(style.substr(1));
                        CHECK(node, "Style \"%s\" not found", style.substr(1).c_str());
                        f = factory.decorate<T>(f, node);
                    } else {
                        const auto [key, value] = style.cut(':');
                        f = factory.decorate<T>(f, key, value ? value.value() : "");
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

        std::unordered_map<String, std::function<sp<Builder<T>>(BeanFactory&, const String&)>> _values;
        std::unordered_map<String, std::function<sp<Builder<T>>(BeanFactory&, const document&)>> _builders;
        std::unordered_map<String, std::function<sp<Builder<T>>(BeanFactory&, const sp<Builder<T>>&, const String&)>> _decorators;
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
        template<typename T> sp<Worker<T>> ensureWorker() {
            if(!_workers.has<Worker<T>>())
                _workers.put<Worker<T>>(sp<Worker<T>>::make(_references, _document_by_id));
            return _workers.get<Worker<T>>();
        }

    private:
        WeakPtr<Scope> _references;
        sp<Dictionary<document>> _document_by_id;

        Traits _workers;
    };

    struct Stub {
        Stub();

        sp<Scope> _references;
        std::list<Factory> _factories;
        std::map<String, sp<BeanFactory>> _packages;
    };

public:
    BeanFactory(std::nullptr_t);
    BeanFactory();
    explicit BeanFactory(sp<Stub> stub);
    BeanFactory(const BeanFactory& other) = default;
    BeanFactory(BeanFactory&& other) = default;

    ~BeanFactory();

    template<typename T> sp<Builder<T>> createBuilderByRef(const Identifier& id) {
        sp<Builder<T>> builder;
        if(id.package()) {
            sp<BeanFactory> factory = getPackage(id.package());
            CHECK(factory, "Id: \"%s\"'s package \"%s\" not found", id.toString().c_str(), id.package().c_str());
            builder = factory ? factory->getBuilderByRef<T>(id.withouPackage(), *this) : nullptr;
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
        const sp<T> obj = build<T>(value, args);
        CHECK(obj, "Counld not build \"%s\"", value.c_str());
        return obj;
    }

    template<typename T> sp<T> ensureByTypeValue(const String& type, const String& value, const Scope& args) {
        return ensureBuilderByTypeValue<T>(type, value)->build(args);
    }

    template<typename T> sp<T> ensure(const document& doc, const Scope& args) {
        const sp<T> obj = build<T>(doc, args);
        CHECK(obj, "Counld not build \"%s\"", Documents::toString(doc).c_str());
        return obj;
    }

    template<typename T> sp<T> ensure(const document& doc, const String& attr, const Scope& args) {
        const sp<T> obj = build<T>(doc, attr, args);
        CHECK(obj, "Counld not build \"%s\" from \"%s\"", attr.c_str(), Documents::toString(doc).c_str());
        return obj;
    }

    template<typename T, typename U> sp<T> buildDecorated(const document& doc, const Scope& args) {
        const sp<Builder<U>> builder = findBuilderByDocument<U>(doc, true);
        if(!builder)
            return nullptr;

        if(const String style = Documents::getAttribute(doc, constants::STYLE))
            return decorate<T>(sp<typename Builder<T>::template Wrapper<Builder<U>>>::make(builder), style)->build(args);
        return builder->build(args);
    }

    template<typename T, typename U> sp<T> ensureDecorated(const document& doc, const Scope& args) {
        const sp<T> obj = buildDecorated<T, U>(doc, args);
        CHECK(obj, "Counld not build \"%s\"", Documents::toString(doc).c_str());
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

    template<typename T> std::vector<builder<T>> makeBuilderList(const document& doc, const String& nodeName) {
        std::vector<builder<T>> list;
        if(const String attrValue = Documents::getAttribute(doc, nodeName))
            list.push_back(ensureBuilder<T>(attrValue));

        for(const document& i : doc->children(nodeName))
            list.push_back(ensureBuilder<T>(i));
        return list;
    }
    template<typename T, typename... Args> std::vector<T> makeBuilderListObject(const document& doc, const String& nodeName, Args&&... args) {
        std::vector<T> list;
        for(const document& i : nodeName ? doc->children(nodeName) : doc->children())
            list.emplace_back(*this, i, std::forward<Args>(args)...);
        return list;
    }
//TODO: deprecate this one, which brings chaos
    template<typename T> sp<Builder<T>> getConcreteClassBuilder(const document& doc, const String& attr) {
        static_assert(!std::is_abstract_v<T>, "Not a concrete class");
        const String attrValue = Documents::getAttribute(doc, attr);
        if(attrValue.empty()) {
            const document& child = doc->getChild(attr);
            return findBuilderByDocument<T>(child ? child : doc, "", false);
        }
        return ensureBuilder<T>(attrValue);
    }

    template<typename T> sp<Builder<T>> getBuilderByRef(const Identifier& id) {
        return getBuilderByRef<T>(id, *this);
    }

    template<typename T> sp<Builder<T>> getBuilderByArg(String argname);
    template<typename T> sp<Builder<T>> getBuilderByArg(const Identifier& id);
    template<typename T> sp<Builder<T>> getBuilderByRef(const Identifier& id, BeanFactory& factory);

    template<typename T> sp<Builder<T>> ensureBuilder(const String& id, Identifier::Type idType = Identifier::ID_TYPE_AUTO) {
        DCHECK(id, "Empty value being built");
        const sp<Builder<T>> builder = getBuilder<T>(id, idType);
        CHECK(builder, "Cannot find builder \"%s\"", id.c_str());
        return builder;
    }

    template<typename T> sp<Builder<T>> ensureBuilder(const document& doc) {
        const sp<Builder<T>> builder = findBuilderByDocument<T>(doc, true);
        CHECK(builder, "Cannot not build from \"%s\"", Documents::toString(doc).c_str());
        return builder;
    }

    template<typename T> sp<Builder<T>> ensureBuilder(const document& doc, const String& attr) {
        const sp<Builder<T>> builder = getBuilder<T>(doc, attr);
        CHECK(builder, "Cannot not build \"%s\" from \"%s\"", attr.c_str(), Documents::toString(doc).c_str());
        return builder;
    }

    template<typename T> sp<Builder<T>> ensureConcreteClassBuilder(const document& doc, const String& attr) {
        const sp<Builder<T>> builder = getConcreteClassBuilder<T>(doc, attr);
        CHECK(builder, "Cannot not build \"%s\" from \"%s\"", attr.c_str(), Documents::toString(doc).c_str());
        return builder;
    }

    template<typename T> sp<Builder<T>> ensureBuilderByTypeValue(const String& type, const String& value) {
        const sp<Builder<T>> builder = findBuilderByTypeValue<T>(type, value);
        CHECK(builder, "Cannot not build Type(%s) with value \"%s\"", type.c_str(), value.c_str());
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

    void add(const Factory& factory, bool front = false);
    void addPackage(const String& name, const BeanFactory& package);

    void extend(const BeanFactory& other);
    sp<BeanFactory> getPackage(const String& name) const;

    const sp<Scope>& references() const;

    template<typename T> sp<Builder<T>> getNullBuilder() const {
        static const sp<Builder<T>> nb = sp<typename Builder<T>::Null>::make();
        return nb;
    }

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

    template<typename T> sp<Builder<T>> decorate(const sp<Builder<T>>& builder, const String& style, const String& value) {
        for(const Factory& i : _stub->_factories)
            if(sp<Builder<T>> f = i.decorate<T>(*this, builder, style, value))
                return f;
        return builder;
    }

    template<typename T> sp<T> buildSafe(const sp<Builder<T>>& builder, const Scope& args) {
        return builder ? builder->build(args) : nullptr;
    }

private:
    sp<Stub> _stub;
};

template<typename T> class BuilderBySoftRef final : public Builder<T> {
public:
    BuilderBySoftRef(String name, const WeakPtr<Scope>& references, sp<Builder<T>> delegate)
        : _name(std::move(name)), _references(references), _delegate(std::move(delegate)) {
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
        return _fallback->build(args);
    }

private:
    WeakPtr<Scope> _references;
    String _name;
    sp<Builder<T>> _fallback;
};

template<typename T> sp<Builder<T>> BeanFactory::Worker<T>::makeWrappedBuilder(sp<Builder<T>> builder, const String& id) const {
    if(id.empty())
        return builder;
    if(id.at(0) == '@')
        return sp<BuilderBySoftRef<T>>::make(id.substr(1), _references, std::move(builder));
    return builder;
}

template<typename T> sp<Builder<T>> BeanFactory::getBuilderByArg(String argname) {
    return sp<BuilderByArgument<T>>::make(_stub->_references, std::move(argname));
}

template<typename T> sp<Builder<T>> BeanFactory::getBuilderByArg(const Identifier& id) {
    CHECK(id.isArg(), "Cannot build \"%s\" because it's not an argument", id.toString().c_str());
    return sp<BuilderByArgument<T>>::make(_stub->_references, id.arg(), findBuilderByValue<T>(id.toString()));
}

template<typename T> sp<Builder<T>> BeanFactory::getBuilderByRef(const Identifier& id, BeanFactory& factory) {
    const String refid = id.ref();
    if(sp<T> inst = _stub->_references->get(refid).template as<T>())
       return sp<typename Builder<T>::Prebuilt>::make(std::move(inst));

    for(const Factory& i : _stub->_factories)
        if(const sp<Builder<T>> builder = i.findBuilder<T>(refid, *this))
            return builder;

    return nullptr;
}

}
