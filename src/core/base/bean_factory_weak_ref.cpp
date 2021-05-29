#include "core/base/bean_factory_weak_ref.h"

#include "core/base/bean_factory.h"

namespace ark {

struct BeanFactoryWeakRef::WeakRef {
    WeakRef(const sp<BeanFactory::Stub>& beanFactory)
        : _bean_factory(beanFactory) {
    }

    WeakPtr<BeanFactory::Stub> _bean_factory;
};

BeanFactoryWeakRef::BeanFactoryWeakRef(const BeanFactory& beanFactory)
    : _weak_ref(sp<WeakRef>::make(beanFactory._stub)) {
}

BeanFactory BeanFactoryWeakRef::ensure() const {
    return BeanFactory(_weak_ref->_bean_factory.ensure());
}

}
