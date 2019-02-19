#ifndef ARK_PLATFORM_ANDROID_IMPL_APPLICATION_ANDROID_APPLICATION_H_
#define ARK_PLATFORM_ANDROID_IMPL_APPLICATION_ANDROID_APPLICATION_H_

#include <android_native_app_glue.h>

#include "app/base/application.h"

namespace ark {
namespace platform {
namespace android {

class AndroidApplication : public Application {
public:
    AndroidApplication(const sp<ApplicationDelegate>& applicationDelegate, const sp<ApplicationContext>& applicationContext, uint32_t width, uint32_t height, const Viewport& viewport);

    virtual int run() override;
    virtual const sp<ApplicationController>& controller() override;

private:
    sp<ApplicationController> _controller;
};

}
}
}

#endif
