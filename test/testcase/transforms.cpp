
#include "test/base/test_case.h"

namespace ark {
namespace unittest {

class TransformsTestCase : public TestCase {
public:
    virtual int launch() {
        return 0;
    }
};

}
}


ark::unittest::TestCase* transforms_create() {
    return new ark::unittest::TransformsTestCase();
}
