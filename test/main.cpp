#include <stdlib.h>

#include "core/ark.h"
#include "core/types/shared_ptr.h"

#include "app/base/application_manifest.h"

using namespace ark;

int main(int argc, const char* argv[])
{
    try {
        Ark ark(argc, argv);
        ark.initialize(sp<ApplicationManifest>::make("manifest.xml"));
        return ark.runTests();
    }
    catch(const std::exception& ex)
    {
        puts(ex.what());
        return -1;
    }
}
