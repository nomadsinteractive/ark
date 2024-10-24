#pragma once

#include "core/base/string.h"
#include "core/collection/table.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Queries {
public:
    Queries(const BeanFactory& factory, Table<String, String> queries);

    BeanFactory ensureBeanFactory() const;

private:
    struct BeanFactoryWeakRef;

    sp<BeanFactoryWeakRef> _bean_factory;
    Table<String, String> _queries;

    friend class BeanFactory;
    friend class Scope;
};

}
