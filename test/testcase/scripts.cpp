
#include "test/base/test_case.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/plugin_manager.h"
#include "core/base/scope.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"
#include "core/inf/dictionary.h"
#include "core/inf/readable.h"
#include "core/inf/interpreter.h"
#include "core/types/shared_ptr.h"
#include "core/util/strings.h"
#include "core/types/global.h"

#include "app/base/application_context.h"
#include "core/base/resource_loader.h"

#include "platform/platform.h"

namespace ark {
namespace unittest {

class ScriptsTestCase : public TestCase {
public:
    virtual int launch() override {
        Global<PluginManager> pluginManager;
        pluginManager->load("ark-python");

        const sp<ResourceLoader> resourceLoader = getResourceLoader();
        if(!resourceLoader)
            return -1;

        Scope args;
        const sp<Interpreter> script = resourceLoader->load<Interpreter>("script", args);
        if(!script) {
            puts("No script interpreter installed");
            return -1;
        }
        const sp<Readable> readable = Ark::instance().openAsset("hello.py");
        if(!readable) {
            puts("Cannot find hello.py");
            return -1;
        }
        const sp<Scope> vars = sp<Scope>::make();
        vars->put("_resource_loader", resourceLoader);
        script->execute(Ark::instance().getAsset("hello.py"), vars);
        return 0;
    }
};

}
}

ark::unittest::TestCase* scripts_create()
{
    return new ark::unittest::ScriptsTestCase();
}
