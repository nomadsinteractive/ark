#include "core/types/shared_ptr.h"

#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/inf/dictionary.h"
#include "core/inf/variable.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"

#include "test/base/test_case.h"
#include "test/base/ref_counter.h"

#include "platform/platform.h"

namespace ark::unittest {

namespace {

class BuilderImpl1 : public IBuilder<uint8_t>, public RefCounter<BuilderImpl1> {
public:
    ~BuilderImpl1() override {
    }

    uint8_t build(const Scope& /*args*/) override {
        return 1;
    }
};

class BuilderImpl2 : public IBuilder<uint16_t>, public RefCounter<BuilderImpl2> {
public:
    uint16_t build(const Scope& /*args*/) override {
        return 2;
    }
};

class BuilderImpl3 : public IBuilder<uint32_t>, public RefCounter<BuilderImpl3> {
public:
    uint32_t build(const Scope& /*args*/) override {
        return 3;
    }
};

class DictionaryImpl : public Dictionary<document> {
public:
    virtual document get(const String& /*name*/) override {
        return document::make("name");
    }
};

}

class LinearBuilder : public Builder<Numeric> {
public:
    virtual sp<Numeric> build(const Scope& /*args*/) override {
        return sp<Numeric::Impl>::make(1.0f);
    }
};

class BeanFactoriesTestCase : public TestCase {
public:
    int launch() override {
        {
            const sp<BeanFactory> beanFactory = sp<BeanFactory>::make(sp<DictionaryImpl>::make());
            BeanFactory::Factory factory;
            factory.addBuilderFactory<uint8_t>([](BeanFactory&, const document&) {return sp<BuilderImpl1>::make();});
            factory.addBuilderFactory<uint16_t>([](BeanFactory&, const document&) {return sp<BuilderImpl2>::make();});
            factory.addBuilderFactory<uint32_t>([](BeanFactory&, const document&) {return sp<BuilderImpl3>::make();});
            beanFactory->add(factory);
            beanFactory->addPackage("self", beanFactory);
            Scope args;
            if((*beanFactory->build<uint8_t>("@a", args).get()) != 1)
                return 1;
            if((*beanFactory->build<uint16_t>("@a", args).get()) != 2)
                return 2;
            if((*beanFactory->build<uint32_t>("@a", args).get()) != 3)
                return 3;
            if((*beanFactory->build<uint8_t>("1", args).get()) != 1)
                return 4;
            if((*beanFactory->build<uint16_t>("2", args).get()) != 2)
                return 5;
            if((*beanFactory->build<uint32_t>("3", args).get()) != 3)
                return 6;
            if((*beanFactory->build<uint32_t>("@self:1", args).get()) != 3)
                return 7;
            if((*beanFactory->build<uint8_t>("@self:1", args).get()) != 1)
                return 8;
        }
        return BuilderImpl1::refCount() == 0 && BuilderImpl2::refCount() == 0 && BuilderImpl3::refCount() == 0 ? 0 : -4;
    }
};

}


ark::unittest::TestCase* bean_factories_create() {
    return new ark::unittest::BeanFactoriesTestCase();
}
