#ifndef ARK_CORE_BASE_QUERIES_H_
#define ARK_CORE_BASE_QUERIES_H_

#include "core/base/bean_factory_weak_ref.h"
#include "core/base/string.h"
#include "core/collection/table.h"

namespace ark {

class ARK_API Queries {
public:
    Queries(const BeanFactory& factory, Table<String, String> queries);

private:
    BeanFactoryWeakRef _bean_factory;
    Table<String, String> _queries;

    friend class BeanFactory;
    friend class Scope;
};

}

#endif
