#include "core/util/math.h"

#include "test/base/test_case.h"

namespace ark {
namespace unittest {

class MathTestCase : public TestCase {
public:
    virtual int launch() {
        TESTCASE_VALIDATE(Math::modFloor(8.0f, 3.0f) == 6.0f);
        return 0;
    }
};

}
}


ark::unittest::TestCase* test_math_create() {
    return new ark::unittest::MathTestCase();
}
