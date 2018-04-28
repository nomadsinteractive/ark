#include <stdlib.h>

#include "core/ark.h"
#include "core/dom/document.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/viewport.h"
#include "graphics/base/size.h"

#include "app/base/application_delegate_impl.h"
#include "app/base/application_manifest.h"
#include "app/impl/application/sdl_application.h"

using namespace ark;

int main(int argc, const char* argv[])
{
    float scale = argc > 1 ? static_cast<float>(atof(argv[1])) : 0.0f;
    if(scale == 0.0f)
        scale = 1.0f;
    try {
        const Ark ark(argc, argv, "manifest.xml");
        const document appManifest = ark.manifest()->getChild("application");
        NOT_NULL(appManifest);
        const sp<ApplicationManifest> applicationManifest = sp<ApplicationManifest>::make(appManifest);
        const sp<Size>& renderResolution = applicationManifest->renderResolution();
        const Viewport viewport(0, 0, renderResolution->width(), renderResolution->height(), 0, renderResolution->width());
        SDLApplication app(sp<ApplicationDelegateImpl>::make(applicationManifest), ark.applicationContext(), (int32_t) (renderResolution->width() * scale), (int32_t) (renderResolution->height() * scale), viewport, Application::WINDOW_FLAG_SHOW_CURSOR);
        return app.run();
    }
    catch(const std::exception& ex)
    {
        puts(ex.what());
        return -1;
    }
}
