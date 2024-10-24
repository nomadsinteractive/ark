#include "core/base/queries.h"

#include "core/base/bean_factory.h"

namespace ark {

struct Queries::BeanFactoryWeakRef {
    BeanFactoryWeakRef(sp<BeanFactory::Stub> beanFactory)
        : _factory_stub(std::move(beanFactory)) {
    }

    WeakPtr<BeanFactory::Stub> _factory_stub;
};

Queries::Queries(const BeanFactory& factory, Table<String, String> queries)
    : _bean_factory(sp<BeanFactoryWeakRef>::make(factory._stub)), _queries(std::move(queries))
{
}

BeanFactory Queries::ensureBeanFactory() const
{
    return BeanFactory(_bean_factory->_factory_stub.ensure());
}

}
