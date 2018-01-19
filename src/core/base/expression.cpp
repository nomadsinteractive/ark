#include "core/base/expression.h"

namespace ark {

bool Expression::isBuildable(const String& expr)
{
    const char* str = expr.c_str();
    return (*str == '$' || *str == '@' ) && Strings::isVariableName(str + 1);
}

}
