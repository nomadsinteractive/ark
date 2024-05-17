#include <stdlib.h>
#include <filesystem>
#include <iostream>
#include <variant>

#include "core/ark.h"
#include "core/types/shared_ptr.h"

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
        const sp<ApplicationManifest> manifest = sp<ApplicationManifest>::make("manifest.xml");
        const Ark ark(argc, argv, manifest);
        const sp<Size>& renderResolution = manifest->rendererResolution();
        SDLApplication app(sp<ApplicationDelegateImpl>::make(manifest), ark.applicationContext(), static_cast<uint32_t>(renderResolution->widthAsFloat() * scale),
                           static_cast<uint32_t>(renderResolution->heightAsFloat() * scale), manifest);
        return app.run();
    }
    catch(const std::exception& ex)
    {
        puts(ex.what());
        return -1;
    }
}
