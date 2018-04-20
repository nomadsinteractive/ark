#ifndef ARK_TEST_BASE_TEST_CASE_H_
#define ARK_TEST_BASE_TEST_CASE_H_

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "app/forwarding.h"

namespace ark {
namespace unittest {

class TestCase {
public:
    virtual ~TestCase() = default;
    virtual int launch() = 0;

    sp<BeanFactory> getBeanFactory() const;
    sp<ResourceLoader> getResourceLoader() const;

};

}
}

#define TESTCASE_VALIDATE(x) if(!(x)) return __LINE__

#endif
