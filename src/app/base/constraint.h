#pragma once

#include "core/types/box.h"

namespace ark {

class Constraint {
public:
//  [[script::bindings::enumeration]]
    enum Type {
        TYPE_FIXED,
        TYPE_GEAR,
        TYPE_HINGE,
        TYPE_SLIDE
    };

    Constraint(Box implementation);

private:
    Box _implementation;
};

}
