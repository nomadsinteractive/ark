#ifndef ARK_PLUGIN_BOX2D_IMPL_JOINT_H_
#define ARK_PLUGIN_BOX2D_IMPL_JOINT_H_

#include <Box2D/Box2D.h>

#include "core/types/shared_ptr.h"

#include "box2d/api.h"
#include "box2d/forwarding.h"

namespace ark {
namespace plugin {
namespace box2d {

class ARK_PLUGIN_BOX2D_API Joint {
public:
    struct Stub {
        sp<World> _world;
        b2Joint* _joint;

        Stub(const sp<World>& world, b2Joint* joint);
        ~Stub();

        void dispose();
        void release();
    };

public:
    Joint(const sp<World>& world, b2Joint* joint);
    ~Joint();

    void dispose();
    void release();

private:
    sp<Stub> _stub;
};

}
}
}

#endif
