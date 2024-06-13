#pragma once

#include "core/base/expectation.h"

namespace ark {

typedef Expectation<int32_t> ExpectationI;

/*

//[[script::bindings::extends(Integer)]]
class ARK_API ExpectationI : public Integer, public Wrapper<Integer> {

//  [[script::bindings::property]]
    const sp<Observer>& observer() const;

};

*/

}
