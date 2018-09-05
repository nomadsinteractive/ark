#ifndef ARK_PLUGIN_BOX2D_IMPL_JOINT_H_
#define ARK_PLUGIN_BOX2D_IMPL_JOINT_H_

#include <Box2D/Box2D.h>

#include "core/types/shared_ptr.h"

#include "box2d/forwarding.h"

namespace ark {
namespace plugin {
namespace box2d {

class Joint {
public:
    Joint(const sp<World>& world, b2Joint* joint);
    ~Joint();

    b2Joint* object();

    template<typename T> T* object() {
        return reinterpret_cast<T*>(_joint);
    }

    void clear();

private:
    sp<World> _world;
    b2Joint* _joint;

};

}
}
}

#endif
