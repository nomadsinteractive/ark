#include "core/base/scope.h"

#include "core/base/string.h"

namespace ark {

Scope::Scope(std::map<String, Box> variables, sp<Queries> queries)
    : _variables(std::move(variables)), _queries(std::move(queries))
{
}

Box Scope::get(const String& name)
{
    return getObject(name);
}

void Scope::put(const String& name, Box value)
{
    _variables[name] = std::move(value);
}

const std::map<String, Box>& Scope::variables() const
{
    return _variables;
}

Box Scope::getObject(const String& name) const
{
    const auto iter = _variables.find(name);
    return iter != _variables.end() ? iter->second : Box();
}

Scope::Queries::Queries(BeanFactory factory, Table<String, String> queries)
    : _bean_factory(std::move(factory)), _queries(std::move(queries))
{
}

}
