#include "core/types/shared_ptr.h"

#include "core/base/callable.h"
#include "core/base/library.h"
#include "core/util/string_convert.h"

#include "test/base/test_case.h"

namespace ark {
namespace unittest {

int test(int i) {
    return i + 1;
}

class CallablesTestCase : public TestCase {
public:
    virtual int launch() override {
        TESTCASE_VALIDATE(StringConvert::eval<uint32_t>("16") != 16);
        TESTCASE_VALIDATE(StringConvert::eval<int32_t>("7") != 7);
        TESTCASE_VALIDATE(StringConvert::eval<int16_t>("3") != 3);

        Library library;
        library.addCallable<int(int)>("test", &test);
        const sp<Callable<int(int)>> callable = library.getCallable<int(int)>("test");
        TESTCASE_VALIDATE(callable->call(0) != 1);

        return 0;
    }
};

}
}


ark::unittest::TestCase* callables_create() {
    return new ark::unittest::CallablesTestCase();
}
