#ifndef ARK_PLATFORM_ANDROID_IMPL_APPLICATION_ANDROID_APPLICATION_H_
#define ARK_PLATFORM_ANDROID_IMPL_APPLICATION_ANDROID_APPLICATION_H_

#include <jni.h>

#include "app/base/application.h"

namespace ark {
namespace platform {
namespace android {

class AndroidApplication : public Application {
public:
    AndroidApplication(const sp<ApplicationDelegate>& applicationDelegate, const sp<ApplicationContext>& applicationContext, uint32_t width, uint32_t height, const Viewport& viewport);

    virtual int run() override;
    virtual const sp<ApplicationController>& controller() override;

    virtual void onCreate() override;

private:
    sp<ApplicationController> _controller;

};

struct RenderRequest {
    sp<RenderView> _render_view;
    sp<RenderCommand> _render_command;
    sp<Runnable> _callback;
};

}
}
}

#endif
