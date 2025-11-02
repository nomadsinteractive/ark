#pragma once

#include "core/types/box.h"

namespace ark {

class Constraint {
public:
    Constraint(Box implementation);
    ~Constraint();

//  [[script::bindings::auto]]
    void discard();

private:
    Box _implementation;
};

}
