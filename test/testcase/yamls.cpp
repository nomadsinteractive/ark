#include "core/types/shared_ptr.h"

#include "core/forwarding.h"
#include "core/base/bean_factory.h"
#include "core/inf/string_bundle.h"

#include "graphics/base/font.h"

#include "test/base/test_case.h"

#include "platform/platform.h"

namespace ark {
namespace unittest {

class YAMLTestCase : public TestCase {
public:
    virtual int launch() {
        const sp<BeanFactory> beanFactory = getBeanFactory();
        const sp<Scope> scope = sp<Scope>::make();
        scope->put("locale", Box(sp<String>::make("zh")));
        const sp<StringBundle> zhStringBundle = beanFactory->build<StringBundle>("@string-bundle-001", scope);
        if(zhStringBundle->getString("l001/_locale").value() != "ZH")
            return 1;
        scope->put("locale", Box(sp<String>::make("en")));
        const sp<StringBundle> enStringBundle = beanFactory->build<StringBundle>("@string-bundle-001", scope);
        if(enStringBundle->getString("l001/_locale").value() != "EN")
            return 2;
        return 0;
    }

};

}
}


ark::unittest::TestCase* yamls_create() {
    return new ark::unittest::YAMLTestCase();
}
