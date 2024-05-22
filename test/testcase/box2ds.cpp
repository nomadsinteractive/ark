
#include "test/base/test_case.h"

#include "core/base/bean_factory.h"
#include "core/base/plugin_manager.h"
#include "core/inf/dictionary.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/types/global.h"
#include "core/util/strings.h"

#include "graphics/base/v3.h"

#include "core/base/resource_loader.h"

#include "platform/platform.h"

namespace ark {
namespace unittest {

class Box2dsTestCase : public TestCase {
public:
    virtual int launch() override {
        const Global<PluginManager> pluginManager;
        pluginManager->load("ark-box2d");
        const sp<ResourceLoader> resourceLoader = getResourceLoader();

        Scope args;
        const sp<Collider> world = resourceLoader->load<Collider>("world", args);
        if(!world.isInstance<Runnable>())
            return 1;

        const sp<Runnable> worldRunnable = world.tryCast<Runnable>();
        const sp<Vec3> position = resourceLoader->load<Vec3>("position", args);
        const sp<Numeric> rotation = resourceLoader->load<Numeric>("rotation", args);
        for(uint32_t i = 0; i < 100; ++i)
        {
            Trace<0, 100> _trace;
            worldRunnable->run();
            const V3 p = position->val();
            float x = p.x();
            float y = p.y();
            float angle = rotation->val();
        }
        const V3 p = position->val();
        if(p.x() == 0 && rotation->val() == 0 && p.y() < -1000.0f)
            return 0;
        return 1;
    }
};

}
}

ark::unittest::TestCase* box2ds_create()
{
    return new ark::unittest::Box2dsTestCase();
}
