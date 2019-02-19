#include <stdlib.h>

#include "android_native_app_glue.h"

#include "core/ark.h"
#include "core/base/manifest.h"
#include "core/types/global.h"
#include "core/types/shared_ptr.h"
#include "core/util/log.h"

#include "graphics/base/viewport.h"
#include "graphics/base/size.h"

#include "app/base/application_delegate_impl.h"

#include "impl/application/android_application.h"
#include "impl/platform_android.h"

using namespace ark;
using namespace ark::platform::android;

void android_main(android_app* state)
{
    Ark ark(0, nullptr);

    const Global<PlatformAndroid> platform(state);

    const sp<Manifest> manifest = sp<Manifest>::make("manifest.xml");
    ark.initialize(manifest);
    
    const sp<Size>& renderResolution = manifest->rendererResolution();
    const Viewport viewport(0, 0, renderResolution->width(), renderResolution->height(), 0, renderResolution->width());
    AndroidApplication app(sp<ApplicationDelegateImpl>::make(manifest), ark.applicationContext(), static_cast<uint32_t>(renderResolution->width()), static_cast<uint32_t>(renderResolution->height()), viewport);
    app.run();
}
