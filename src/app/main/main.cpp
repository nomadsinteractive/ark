#include "core/ark.h"
#include "core/types/shared_ptr.h"

#include "app/base/application.h"
#include "app/base/application_manifest.h"

using namespace ark;

int main(const int32_t argc, const char* argv[])
{
    Ark ark(argc, argv);
    try {
        ark.initialize();
        const sp<Application> app = ark.makeApplication();
        return app->run();
    }
    catch(const std::exception& ex)
    {
        puts(ex.what());
        return -1;
    }
}
