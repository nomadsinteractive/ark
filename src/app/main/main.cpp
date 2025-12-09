#include "core/ark.h"
#include "core/types/shared_ptr.h"

#include "app/base/application.h"
#include "app/base/application_manifest.h"

#if defined(ARK_FLAG_PUBLISHING_BUILD) && defined(WIN32)
#include <windows.h>

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, const int32_t nShowCmd)
{
    const int32_t argc = __argc;
    const char** argv = const_cast<const char**>(__argv);
#else
int main(const int32_t argc, const char* argv[])
{
#endif
    using namespace ark;

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
