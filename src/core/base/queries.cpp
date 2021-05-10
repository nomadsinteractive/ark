#include "core/base/queries.h"

namespace ark {

Queries::Queries(BeanFactory factory, Table<String, String> queries)
    : _bean_factory(std::move(factory)), _queries(std::move(queries))
{
}

}
