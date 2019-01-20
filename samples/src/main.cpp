#include <stdlib.h>

#include "core/ark.h"
#include "core/base/manifest.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/viewport.h"
#include "graphics/base/size.h"

#include "app/base/application_delegate_impl.h"
#include "app/impl/application/sdl_application.h"

using namespace ark;

int main(int argc, const char* argv[])
{
    float scale = argc > 1 ? static_cast<float>(atof(argv[1])) : 0.0f;
    if(scale == 0.0f)
        scale = 1.0f;
    try {
        const sp<Manifest> manifest = sp<Manifest>::make("manifest.xml");
        const Ark ark(argc, argv, manifest);
        const sp<Size>& renderResolution = manifest->rendererResolution();
        const Viewport viewport(0, 0, renderResolution->width(), renderResolution->height(), 0, renderResolution->width());
        SDLApplication app(sp<ApplicationDelegateImpl>::make(manifest), ark.applicationContext(), (int32_t) (renderResolution->width() * scale), (int32_t) (renderResolution->height() * scale), viewport, Application::WINDOW_FLAG_SHOW_CURSOR);
        return app.run();
    }
    catch(const std::exception& ex)
    {
        puts(ex.what());
        return -1;
    }
}
