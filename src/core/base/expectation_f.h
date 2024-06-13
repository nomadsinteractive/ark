#pragma once

#include "core/base/expectation.h"

namespace ark {

typedef Expectation<float> ExpectationF;

/*

//[[script::bindings::extends(Numeric)]]
class ARK_API ExpectationF : public Numeric, public Wrapper<Numeric> {

//  [[script::bindings::property]]
    const sp<Observer>& observer() const;
};

*/

}
