#include "core/types/shared_ptr.h"

#include "test/base/test_case.h"
#include "test/base/ref_counter.h"

namespace ark {
namespace unittest {

class TestA : public RefCounter<TestA> {
public:
    TestA() {}
    ~TestA() {
    }
};

class BoxesTestCase : public TestCase {
public:
    virtual int launch() override {
        {
            sp<TestA> i32 = sp<TestA>::make();
            Box box(i32.pack());
            box.unpack<TestA>();
        }
        return TestA::refCount() == 0 ? 0 : 1;
    }
};

}
}


ark::unittest::TestCase* boxes_create() {
    return new ark::unittest::BoxesTestCase();
}
