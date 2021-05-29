#include "core/base/queries.h"

namespace ark {

Queries::Queries(const BeanFactory& factory, Table<String, String> queries)
    : _bean_factory(factory), _queries(std::move(queries))
{
}

}
