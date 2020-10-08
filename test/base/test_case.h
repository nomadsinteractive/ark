#ifndef ARK_TEST_BASE_TEST_CASE_H_
#define ARK_TEST_BASE_TEST_CASE_H_

#include "core/forwarding.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "app/forwarding.h"

namespace ark {
namespace unittest {

class TestCase {
public:
    TestCase(const String& factoryFile = "application.xml");
    virtual ~TestCase() = default;

    virtual int launch() = 0;

    sp<BeanFactory> getBeanFactory() const;
    sp<ResourceLoader> getResourceLoader() const;

private:
    String _factory_file;
};

}
}

#define TESTCASE_VALIDATE(x) while(!(x)) return __LINE__

#endif
