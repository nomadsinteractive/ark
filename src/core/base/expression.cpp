#include "core/base/expression.h"

#include <regex>

#include "core/impl/numeric/add.h"
#include "core/impl/numeric/subtract.h"
#include "core/impl/numeric/multiply.h"
#include "core/impl/numeric/divide.h"
#include "core/inf/array.h"

namespace ark {

bool Expression::isBuildable(const String& expr)
{
    const char* str = expr.c_str();
    return (*str == '$' || *str == '@' ) && Strings::isVariableName(str + 1);
}

}
