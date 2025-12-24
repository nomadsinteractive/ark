#include "core/base/scope.h"

namespace ark {

Scope::Scope(Table<String, Box> variables)
    : _variables(std::move(variables))
{
}

Box Scope::get(const String& name)
{
    Optional<Box> optVar = getObject(name);
    return optVar ? optVar.value() : Box();
}

void Scope::put(const String& name, Box value)
{
    _variables[name] = std::move(value);
}

const Table<String, Box>& Scope::variables() const
{
    return _variables;
}

Optional<Box> Scope::getObject(const String& name) const
{
    const auto iter = _variables.find(name);
    return iter != _variables.end() ? Optional<Box>{iter->second} : Optional<Box>{};
}

}
