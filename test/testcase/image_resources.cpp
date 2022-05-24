#include "core/types/shared_ptr.h"

#include "core/ark.h"
#include "core/impl/dictionary/loader_bundle.h"

#include "graphics/base/bitmap.h"

#include "app/base/application_context.h"
#include "app/base/application_bundle.h"

#include "test/base/test_case.h"

namespace ark {
namespace unittest {

class ImageResourcesTestCase : public TestCase {
public:
    virtual int launch() {
        const sp<ApplicationBundle> applicationResource = Ark::instance().applicationContext()->applicationBundle();
        const bitmap s001 = applicationResource->bitmapBundle()->get("s001.png");
        if(!s001) {
            printf("s001.png not found!\n");
            return -1;
        }
        TESTCASE_VALIDATE(s001->width() == 256);
        TESTCASE_VALIDATE(s001->height() == 256);

        const bitmap s003 = applicationResource->bitmapBundle()->get("s003.jpg");
        TESTCASE_VALIDATE(s003->width() == 780);
        TESTCASE_VALIDATE(s003->height() == 460);
        return 0;
    }
};

}
}


ark::unittest::TestCase* image_resources_create() {
    return new ark::unittest::ImageResourcesTestCase();
}
