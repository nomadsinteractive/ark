#include "app/base/constraint.h"

#include "core/types/global.h"

namespace ark {

namespace {

struct ConstraintKeeper {
    Vector<Box> _constraints;
};

}

Constraint::Constraint(Box implementation)
    : _implementation(std::move(implementation))
{
}

Constraint::~Constraint()
{
    if(_implementation)
        Global<ConstraintKeeper>()->_constraints.push_back(std::move(_implementation));
}

void Constraint::discard()
{
    _implementation = {};
}

}
