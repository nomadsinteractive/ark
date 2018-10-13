#include "core/types/shared_ptr.h"

#include "core/base/bean_factory.h"
#include "core/dom/document.h"
#include "core/inf/builder.h"
#include "core/inf/dictionary.h"
#include "core/inf/variable.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"
#include "core/types/null.h"
#include "core/util/bean_utils.h"

#include "test/base/test_case.h"
#include "test/base/ref_counter.h"

#include "platform/platform.h"

namespace ark {
namespace unittest {

namespace {

class BuilderImpl1 : public Builder<uint8_t>, public RefCounter<BuilderImpl1> {
public:
    ~BuilderImpl1() {
    }

    virtual sp<uint8_t> build(const sp<Scope>& /*args*/) override {
        return sp<uint8_t>::make(1);
    }
};

class BuilderImpl2 : public Builder<uint16_t>, public RefCounter<BuilderImpl2> {
public:
    virtual sp<uint16_t> build(const sp<Scope>& /*args*/) override {
        return sp<uint16_t>::make(2);
    }
};

class BuilderImpl3 : public Builder<uint32_t>, public RefCounter<BuilderImpl3> {
public:
    virtual sp<uint32_t> build(const sp<Scope>& /*args*/) override {
        return sp<uint32_t>::make(3);
    }
};

class DictionaryImpl : public Dictionary<document> {
public:
    virtual document get(const String& /*name*/) override {
        return document::make("name");
    }
};

template<typename T> class ValueDictionaryImpl : public Builder<T> {
public:
    ValueDictionaryImpl(BeanFactory& factory, const String& value)
        : _value(static_cast<T>(BeanUtils::toFloat(factory, value))) {
    }

    virtual sp<T> build(const sp<Scope>& /*args*/) override {
        return sp<T>::make(_value);
    }

private:
    T _value;
};

}

class LinearBuilder : public Builder<Numeric> {
public:
    virtual sp<Numeric> build(const sp<Scope>& /*args*/) override {
        return sp<Numeric::Impl>::make(1.0f);
    }
};

class BeanFactoriesTestCase : public TestCase {
public:
    virtual int launch() override {
        {
            const sp<BeanFactory> beanFactory = sp<BeanFactory>::make();
            BeanFactory::Factory factory(beanFactory->references(), sp<DictionaryImpl>::make());
            factory.addBuilderFactory<uint8_t>([](BeanFactory&, const document&) {return sp<BuilderImpl1>::make();});
            factory.addBuilderFactory<uint16_t>([](BeanFactory&, const document&) {return sp<BuilderImpl2>::make();});
            factory.addBuilderFactory<uint32_t>([](BeanFactory&, const document&) {return sp<BuilderImpl3>::make();});
            factory.addDictionaryFactory<uint8_t>([](BeanFactory& factory, const String& value) {return sp<ValueDictionaryImpl<uint8_t>>::make(factory, value);});;
            factory.addDictionaryFactory<uint16_t>([](BeanFactory& factory, const String& value) {return sp<ValueDictionaryImpl<uint16_t>>::make(factory, value);});;
            factory.addDictionaryFactory<uint32_t>([](BeanFactory& factory, const String& value) {return sp<ValueDictionaryImpl<uint32_t>>::make(factory, value);});;
            beanFactory->add(factory);
            beanFactory->addPackage("self", beanFactory);
            if((*beanFactory->build<uint8_t>("@a").get()) != 1)
                return 1;
            if((*beanFactory->build<uint16_t>("@a").get()) != 2)
                return 2;
            if((*beanFactory->build<uint32_t>("@a").get()) != 3)
                return 3;
            if((*beanFactory->build<uint8_t>("1").get()) != 1)
                return 4;
            if((*beanFactory->build<uint16_t>("2").get()) != 2)
                return 5;
            if((*beanFactory->build<uint32_t>("3").get()) != 3)
                return 6;
            if((*beanFactory->build<uint32_t>("@self:1").get()) != 3)
                return 7;
            if((*beanFactory->build<uint8_t>("@self:1").get()) != 1)
                return 8;
        }
        return BuilderImpl1::refCount() == 0 && BuilderImpl2::refCount() == 0 && BuilderImpl3::refCount() == 0 ? 0 : -4;
    }
};

}
}


ark::unittest::TestCase* bean_factories_create() {
    return new ark::unittest::BeanFactoriesTestCase();
}
