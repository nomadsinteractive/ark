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
        if(StringConvert::to<String, uint32_t>("16") != 16)
            return 1;
        if(StringConvert::to<String, int32_t>("7") != 7)
            return 2;
        if(StringConvert::to<String, int16_t>("3") != 3)
            return 3;

        Library library;
        library.addCallable<int(int)>("test", &test);
        const sp<Callable<int(int)>> callable = library.getCallable<int(int)>("test");
        if(callable->call(0) != 1)
            return 4;
        return 0;
    }
};

}
}


ark::unittest::TestCase* callables_create() {
    return new ark::unittest::CallablesTestCase();
}
