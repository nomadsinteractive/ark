#ifndef ARK_CORE_BEAN_FACTORY_H_
#define ARK_CORE_BEAN_FACTORY_H_

#include <list>
#include <type_traits>
#include <unordered_map>

#include "core/base/api.h"
#include "core/base/identifier.h"
#include "core/collection/by_type.h"
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
            return createBuilder(factory, manifest);
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

        sp<Builder<T>> createBuilder(BeanFactory& factory, const document& doc) const {
            const String className = Documents::getAttribute(doc, Constants::Attributes::CLASS);
            return createBuilder(factory, className, doc);
        }

        sp<Builder<T>> createBuilder(BeanFactory& factory, const String& className, const document& doc) const {
            const String id = Documents::getAttribute(doc, Constants::Attributes::ID);
            const String ref = Documents::getAttribute(doc, Constants::Attributes::REF);
            const String style = Documents::getAttribute(doc, Constants::Attributes::STYLE);
            if(className.empty()) {
                do {
                    if(ref) {
                        const Identifier f = Identifier::parse(ref);
                        if(f.isRef()) {
                            if(f.package().empty() && (f.ref() == id || ref == id))
                                break;
                            const sp<Builder<T>> builder = factory.createBuilderByRef<T>(f);
                            return wrapBuilder(decorateBuilder(factory, builder, style), id);
                        }
                        if(f.isArg())
                            return decorateBuilder(factory, factory.getBuilderByArg<T>(f), style);
                    }
                    return createBuilderInternal(factory, doc->name(), style, id, doc);
                } while(false);
            }
            DWARN(ref.empty() || ref.at(0) == '@', "Building class \"%s\" with reference \"%s\" has no effect", className.c_str(), ref.c_str());
            return createBuilderInternal(factory, className, style, ref ? ref : id, doc);
        }

    private:
        sp<Builder<T>> createBuilderInternal(BeanFactory& factory, const String& className, const String& style, const String& id, const document& doc) const {
            const auto iter = _builders.find(className);
            if(iter == _builders.end())
                return _default_builder_factory ? wrapBuilder(decorateBuilder(factory, _default_builder_factory(factory, doc), style), id) : nullptr;
            const sp<Builder<T>> builder = iter->second(factory, doc);
            DCHECK(builder, "Builder \"%s\" create failed", className.c_str());
            return wrapBuilder(decorateBuilder(factory, builder, style), id);
        }

        sp<Builder<T>> wrapBuilder(sp<Builder<T>> builder, const String& id) const;

        sp<Builder<T>> decorateBuilder(BeanFactory& factory, const sp<Builder<T>>& builder, const String& styles) const {
            if(styles && builder) {
                sp<Builder<T>> f = builder;
                for(const String& style : styles.split(';')) {
                    if(style.at(0) == '@') {
                        const document node = _document_by_id->get(style.substr(1));
                        DCHECK(node, "Style \"%s\" not found", style.substr(1).c_str());
                        f = factory.decorate<T>(f, node);
                    } else {
                        String key, value;
                        Strings::parseNameValuePair(style, ':', key, value);
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

    struct Stub {
        Stub();

        sp<Scope> _references;
        std::list<Factory> _factories;
        std::map<String, sp<BeanFactory>> _packages;
    };

public:
    BeanFactory(std::nullptr_t);
    BeanFactory();
    BeanFactory(const BeanFactory& other) = default;
    BeanFactory(BeanFactory&& other) = default;
    ~BeanFactory();

    template<typename T> sp<Builder<T>> createBuilderByRef(const Identifier& id) {
        sp<Builder<T>> builder;
        if(id.package()) {
            const sp<BeanFactory>& factory = getPackage(id.package());
            DCHECK(factory, "Id: \"%s\"'s package \"%s\" not found", id.toString().c_str(), id.package().c_str());
            builder = factory ? factory->getBuilderByRef<T>(id, *this) : nullptr;
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
        return buildSafe<T>(findBuilderByDocument<T>(doc), args);
    }

    template<typename T> sp<T> build(const document& doc, const String& attr, const Scope& args) {
        return buildSafe<T>(getBuilder<T>(doc, attr), args);
    }

    template<typename T> sp<T> ensure(const String& value, const Scope& args) {
        const sp<T> obj = build<T>(value, args);
        DCHECK(obj, "Counld not build \"%s\"", value.c_str());
        return obj;
    }

    template<typename T> sp<T> ensure(const String& type, const String& value, const Scope& args) {
        return ensureBuilderByTypeValue<T>(type, value)->build(args);
    }

    template<typename T> sp<T> ensure(const document& doc, const Scope& args) {
        const sp<T> obj = build<T>(doc, args);
        DCHECK(obj, "Counld not build \"%s\"", Documents::toString(doc).c_str());
        return obj;
    }

    template<typename T> sp<T> ensure(const document& doc, const String& attr, const Scope& args) {
        const sp<T> obj = build<T>(doc, attr, args);
        DCHECK(obj, "Counld not build \"%s\" from \"%s\"", attr.c_str(), Documents::toString(doc).c_str());
        return obj;
    }

    template<typename T, typename U> sp<T> buildDecorated(const document& doc, const Scope& args) {
        const sp<Builder<U>> builder = findBuilderByDocument<U>(doc);
        if(!builder)
            return nullptr;

        const String style = Documents::getAttribute(doc, Constants::Attributes::STYLE);
        if(style)
            return decorate<T>(sp<typename Builder<T>::template Wrapper<U>>::make(builder), style)->build(args);
        return builder->build(args);
    }

    template<typename T, typename U> sp<T> ensureDecorated(const document& doc, const Scope& args) {
        const sp<T> obj = buildDecorated<T, U>(doc, args);
        DCHECK(obj, "Counld not build \"%s\"", Documents::toString(doc).c_str());
        return obj;
    }

    template<typename T> sp<Builder<T>> getBuilder(const String& id) {
        if(id.empty())
            return nullptr;

        const Identifier f = Identifier::parse(id);
        if(!std::is_same<T, String>::value && f.isRef())
            return createBuilderByRef<T>(f);
        if(f.isArg())
            return getBuilderByArg<T>(f);
        return findBuilderByValue<T>(id);
    }

    template<typename T> sp<Builder<T>> getBuilder(const document& doc, const String& attr, const String& defValue = String()) {
        const String attrValue = Documents::getAttribute(doc, attr, defValue);
        if(attrValue.empty()) {
            const document& child = doc->getChild(attr);
            if(child) {
                const sp<Builder<T>> builder = findBuilderByDocument<T>(child);
                DCHECK(builder, "Cannot build \"%s\" from \"%s\"", attr.c_str(), Documents::toString(doc).c_str());
                return builder;
            }
            return nullptr;
        }
        return ensureBuilder<T>(attrValue);
    }

    template<typename T> sp<Builder<T>> getConcreteClassBuilder(const document& doc, const String& attr) {
        static_assert(!std::is_abstract<T>::value, "Not a concrete class");
        const String attrValue = Documents::getAttribute(doc, attr);
        if(attrValue.empty()) {
            const document& child = doc->getChild(attr);
            return findBuilderByDocument<T>(child ? child : doc);
        }
        return ensureBuilder<T>(attrValue);
    }

    template<typename T> sp<Builder<T>> getBuilderByRef(const Identifier& id) {
        return getBuilderByRef<T>(id, *this);
    }

    template<typename T> sp<Builder<T>> getBuilderByArg(const String& argname);
    template<typename T> sp<Builder<T>> getBuilderByArg(const Identifier& id);
    template<typename T> sp<Builder<T>> getBuilderByRef(const Identifier& id, BeanFactory& factory);

    template<typename T> sp<Builder<T>> ensureBuilder(const String& id) {
        DCHECK(id, "Empty value being built");
        const sp<Builder<T>> builder = getBuilder<T>(id);
        DCHECK(builder, "Could find builder \"%s\"", id.c_str());
        return builder;
    }

    template<typename T> sp<Builder<T>> ensureBuilder(const document& doc) {
        const sp<Builder<T>> builder = findBuilderByDocument<T>(doc);
        DCHECK(builder, "Counld not build from \"%s\"", Documents::toString(doc).c_str());
        return builder;
    }

    template<typename T> sp<Builder<T>> ensureBuilder(const document& doc, const String& attr) {
        const sp<Builder<T>> builder = getBuilder<T>(doc, attr);
        DCHECK(builder, "Counld not build \"%s\" from \"%s\"", attr.c_str(), Documents::toString(doc).c_str());
        return builder;
    }

    template<typename T> sp<Builder<T>> ensureConcreteClassBuilder(const document& doc, const String& attr) {
        const sp<Builder<T>> builder = getConcreteClassBuilder<T>(doc, attr);
        DCHECK(builder, "Counld not build \"%s\" from \"%s\"", attr.c_str(), Documents::toString(doc).c_str());
        return builder;
    }

    template<typename T> sp<Builder<T>> ensureBuilderByTypeValue(const String& type, const String& value) {
        const sp<Builder<T>> builder = findBuilderByTypeValue<T>(type, value);
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

    void add(const Factory& factory, bool front = false);
    void addPackage(const String& name, const BeanFactory& package);

    void extend(const BeanFactory& other);
    const sp<BeanFactory>& getPackage(const String& name) const;

    const sp<Scope>& references() const;

    template<typename T> sp<Builder<T>> getNullBuilder() const {
        static const sp<Builder<T>> nb = sp<typename Builder<T>::Null>::make();
        return nb;
    }

    class WeakRef {
    public:
        WeakRef() = default;
        WeakRef(const BeanFactory& other)
            : _stub(other._stub) {
        }

        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(WeakRef);

        BeanFactory lock() const {
            return BeanFactory(_stub.lock());
        }

        BeanFactory ensure() const {
            return BeanFactory(_stub.ensure());
        }

    private:
        WeakPtr<Stub> _stub;
    };

private:
    template<typename T> sp<Builder<T>> findBuilderByDocument(const document& doc) {
        const String className = Documents::getAttribute(doc, Constants::Attributes::CLASS);
        return findBuilderByDocument<T>(doc, className);
    }

    template<typename T> sp<Builder<T>> findBuilderByDocument(const document& doc, const String& className) {
        for(const Factory& i : _stub->_factories) {
            const sp<Builder<T>> builder = i.createBuilder<T>(className, doc, *this);
            if(builder)
                return builder;
        }
        return nullptr;
    }

    template<typename T> sp<Builder<T>> findBuilderByValue(const String& value) {
        for(const Factory& i : _stub->_factories) {
            const sp<Builder<T>> builder = i.createValueBuilder<T>(*this, value);
            if(builder)
                return builder;
        }
        return nullptr;
    }

    template<typename T> sp<Builder<T>> findBuilderByTypeValue(const String& type, const String& value) {
        for(const Factory& i : _stub->_factories) {
            const sp<Builder<T>> builder = i.createValueBuilder<T>(*this, type, value);
            if(builder)
                return builder;
        }
        return nullptr;
    }

    template<typename T> sp<Builder<T>> decorate(const sp<Builder<T>>& builder, const String& style, const String& value) {
        for(const Factory& i : _stub->_factories) {
            const sp<Builder<T>> f = i.decorate<T>(*this, builder, style, value);
            if(f)
                return f;
        }
        return builder;
    }

    template<typename T> sp<T> buildSafe(const sp<Builder<T>>& builder, const Scope& args) {
        return builder ? builder->build(args) : nullptr;
    }
private:
    BeanFactory(sp<Stub> stub)
        : _stub(std::move(stub)) {
    }

private:
    sp<Stub> _stub;

    friend class WeakRef;
};

}

#endif

#ifndef ARK_CORE_BEAN_FACTORY_H_APPENDIX_
#define ARK_CORE_BEAN_FACTORY_H_APPENDIX_

#include "core/base/scope.h"

namespace ark {

namespace  {

template<typename T> class BuilderBySoftRef : public Builder<T> {
public:
    BuilderBySoftRef(String name, const WeakPtr<Scope>& references, sp<Builder<T>> delegate)
        : _name(std::move(name)), _references(references), _delegate(std::move(delegate)) {
    }

    virtual sp<T> build(const Scope& args) override {
        const sp<Scope> reference = _references.lock();
        DCHECK(reference, "BeanFactory has been disposed");
        sp<T> inst = reference->get<T>(_name);
        if(!inst) {
            inst = _delegate->build(args);
            DCHECK(inst, "Cannot build \"%s\"", _name.c_str());
            reference->put(_name, inst);
            _delegate = nullptr;
        }
        return inst;
    }

private:
    String _name;
    WeakPtr<Scope> _references;
    sp<Builder<T>> _delegate;
};

template<typename T> class BuilderByArgument : public Builder<T> {
public:
    BuilderByArgument(const WeakPtr<Scope>& references, const String& name, const sp<Builder<T>> fallback = nullptr)
        : _references(references), _name(name), _fallback(std::move(fallback)) {
    }

    virtual sp<T> build(const Scope& args) override {
        sp<T> value = args.build<T>(_name, args);
        if(!value) {
            const sp<Scope> references = _references.lock();
            DCHECK(references, "BeanFactory has been disposed");
            value = references->get<T>(_name);
        }
        DCHECK(value || _fallback, "Cannot get argument \"%s\"", _name.c_str());
        return value ? value : _fallback->build(args);
    }

private:
    WeakPtr<Scope> _references;
    String _name;
    sp<Builder<T>> _fallback;
};

template<typename T> class BuilderWithQueries : public Builder<T> {
public:
    BuilderWithQueries(sp<Builder<T>> delegate, BeanFactory factory, const Identifier& id)
        : _delegate(std::move(delegate)), _queries(sp<Scope::Queries>::make(std::move(factory), id.queries())) {
    }

    virtual sp<T> build(const Scope& args) override {
        Scope argsAndQueries(args.variables(), _queries);
        return _delegate->build(argsAndQueries);
    }

private:
    sp<Builder<T>> _delegate;
    sp<Scope::Queries> _queries;
};

}

template<typename T> sp<Builder<T>> BeanFactory::Worker<T>::wrapBuilder(sp<Builder<T>> builder, const String& id) const {
    if(id.empty())
        return builder;
    if(id.at(0) == '@')
        return sp<Builder<T>>::adopt(new BuilderBySoftRef<T>(id.substr(1), _references, std::move(builder)));
    return builder;
}

template<typename T> sp<Builder<T>> BeanFactory::getBuilderByArg(const String& argname) {
    return sp<BuilderByArgument<T>>::make(_stub->_references, argname);
}

template<typename T> sp<Builder<T>> BeanFactory::getBuilderByArg(const Identifier& id) {
    DCHECK(id.isArg(), "Cannot build \"%s\" because it's not an argument", id.toString().c_str());
    return sp<BuilderByArgument<T>>::make(_stub->_references, id.arg(), findBuilderByValue<T>(id.toString()));
}

template<typename T> sp<Builder<T>> BeanFactory::getBuilderByRef(const Identifier& id, BeanFactory& factory) {
    String refid = id.ref();
    const sp<T> inst = _stub->_references->get<T>(refid);
    if(inst)
       return sp<typename Builder<T>::Prebuilt>::make(inst);

    for(const Factory& i : _stub->_factories) {
       const sp<Builder<T>> builder = i.findBuilder<T>(refid, *this);
       if(builder) {
           if(id.queries().size())
               return sp<BuilderWithQueries<T>>::make(builder, factory, id);
           return builder;
       }
    }
    return nullptr;
}

}

#endif
