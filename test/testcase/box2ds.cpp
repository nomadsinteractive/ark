
#include "test/base/test_case.h"

#include "core/base/bean_factory.h"
#include "core/base/plugin_manager.h"
#include "core/base/object.h"
#include "core/inf/dictionary.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/types/global.h"
#include "core/util/strings.h"

#include "graphics/base/vec2.h"

#include "platform/platform.h"

namespace ark {
namespace unittest {

class Box2dsTestCase : public TestCase {
public:
    virtual int launch() override {
        const Global<PluginManager> pluginManager;
        pluginManager->load("ark-box2d");

        const sp<BeanFactory> beanFactory = getBeanFactory();
        if(!beanFactory)
            return -1;

        const sp<Object> world = beanFactory->load<Object>("world");
        if(!world.is<Runnable>())
            return 1;
        const sp<Runnable> worldRunnable = world.as<Runnable>();
        const sp<Object> body = beanFactory->load<Object>("body");
        const sp<Vec2> position = beanFactory->load<Vec2>("position");
        const sp<Numeric> rotation = beanFactory->load<Numeric>("rotation");
        for(uint32_t i = 0; i < 100; ++i)
        {
            Trace<0, 100> _trace;
            worldRunnable->run();
            float x = position->x();
            float y = position->y();
            float angle = rotation->val();
        }
        return 0;
    }
};

}
}

ark::unittest::TestCase* box2ds_create()
{
    return new ark::unittest::Box2dsTestCase();
}
