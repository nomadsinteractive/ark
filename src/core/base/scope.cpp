#include "core/base/scope.h"

#include "core/base/string.h"

namespace ark {

Scope::Scope(std::map<String, Box> variables)
    : _variables(std::move(variables))
{
}

Box Scope::get(const String& name)
{
    auto iter = _variables.find(name);
    return iter != _variables.end() ? iter->second : Box();
}

void Scope::put(const String& name, const Box& value)
{
    _variables[name] = value;
}

const std::map<String, Box>& Scope::variables() const
{
    return _variables;
}

std::map<String, Box>& Scope::variables()
{
    return _variables;
}

}
