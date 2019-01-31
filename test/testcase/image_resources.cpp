#include "core/types/shared_ptr.h"

#include "core/ark.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/bitmap_bundle.h"

#include "app/base/application_context.h"
#include "app/base/application_resource.h"

#include "test/base/test_case.h"

namespace ark {
namespace unittest {

class ImageResourcesTestCase : public TestCase {
public:
    virtual int launch() {
        const sp<ApplicationResource> applicationResource = Ark::instance().applicationContext()->applicationResource();
        const bitmap s001 = applicationResource->bitmapBundle()->get("s001.png");
        if(!s001) {
            printf("s001.png not found!\n");
            return -1;
        }
        if(s001->width() != 256)
            return 1;
        if(s001->height() != 256)
            return 2;

        const bitmap s003 = applicationResource->bitmapBundle()->get("s003.jpg");
        if(s003->width() != 780)
            return 3;
        if(s003->height() != 460)
            return 4;
        return 0;
    }
};

}
}


ark::unittest::TestCase* image_resources_create() {
    return new ark::unittest::ImageResourcesTestCase();
}
