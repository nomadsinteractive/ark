#ifndef ARK_CORE_BASE_QUERIES_H_
#define ARK_CORE_BASE_QUERIES_H_

#include "core/base/bean_factory.h"
#include "core/collection/table.h"
#include "core/types/box.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Queries {
public:
    Queries(BeanFactory factory, Table<String, String> queries);

    template<typename T> sp<T> build(const String& name, const Scope& args) {
        const auto iter = _queries.find(name);
        if(iter != _queries.end())
            return _bean_factory.build<T>(iter->second, args);
        return nullptr;
    }

private:
    BeanFactory _bean_factory;
    Table<String, String> _queries;
};

}

#endif
