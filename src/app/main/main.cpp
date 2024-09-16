#include <iostream>

#include "core/ark.h"
#include "core/types/shared_ptr.h"

#include "app/base/application.h"
#include "app/base/application_manifest.h"

using namespace ark;

int main(int argc, const char* argv[])
{
    const float scale = argc > 1 ? static_cast<float>(atof(argv[1])) : 1.0f;
    try {
        Ark ark(argc, argv);
        sp<ApplicationManifest> manifest = sp<ApplicationManifest>::make("manifest.xml");
        const V2& renderResolution = manifest->rendererResolution();
        const sp<Application> app = ark.makeApplication(std::move(manifest), static_cast<uint32_t>(renderResolution.x() * scale), static_cast<uint32_t>(renderResolution.y() * scale));
        return app->run();
    }
    catch(const std::exception& ex)
    {
        puts(ex.what());
        return -1;
    }
}
