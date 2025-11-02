#include "app/base/constraint.h"

namespace ark {

Constraint::Constraint(Box implementation)
    : _implementation(std::move(implementation))
{
}

}
