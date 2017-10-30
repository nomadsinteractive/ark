
#include "test/base/test_case.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/plugin_manager.h"
#include "core/base/scope.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"
#include "core/inf/dictionary.h"
#include "core/inf/readable.h"
#include "core/inf/script.h"
#include "core/types/shared_ptr.h"
#include "core/util/strings.h"
#include "core/types/global.h"

#include "app/base/application_context.h"
#include "app/base/resource_loader.h"

#include "platform/platform.h"

namespace ark {
namespace unittest {

class ScriptsTestCase : public TestCase {
public:
    virtual int launch() override {
        Global<PluginManager> pluginManager;
        pluginManager->load("ark-python");

        const sp<ResourceLoader> resourceLoader = Ark::instance().applicationContext()->createResourceLoader("application.xml");
        if(!resourceLoader)
            return -1;

        const sp<Script> script = resourceLoader->load<Script>("script");
        if(!script) {
            puts("No script interpreter installed");
            return -1;
        }
        const sp<Readable> readable = Ark::instance().getResource("hello.py");
        if(!readable) {
            puts("Cannot find hello.py");
            return -1;
        }
        const sp<Scope> vars = sp<Scope>::make();
        vars->put<ResourceLoader>("_resource_loader", resourceLoader);
        script->run(Strings::loadFromReadable(readable), vars);
        return 0;
    }
};

}
}

ark::unittest::TestCase* scripts_create()
{
    return new ark::unittest::ScriptsTestCase();
}
