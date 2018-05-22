#ifndef ARK_PLUGIN_PYTHON_IMPL_DUCK_PY_OBJECT_DUCK_TYPE_H_
#define ARK_PLUGIN_PYTHON_IMPL_DUCK_PY_OBJECT_DUCK_TYPE_H_

#include "core/inf/duck.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

#include "python/forwarding.h"

namespace ark {
namespace plugin {
namespace python {

class PyObjectDuckType : public Duck<CollisionCallback>, public Duck<Array<Color>>, public Duck<Array<int32_t>>, public Implements<PyObjectDuckType, Duck<CollisionCallback>,
        Duck<Array<Color>>, Duck<Array<int32_t>>> {
public:
    PyObjectDuckType(const sp<PyInstance>& inst);

    virtual void to(sp<CollisionCallback>& inst) override;
    virtual void to(sp<Array<Color>>& inst) override;
    virtual void to(sp<Array<int32_t>>& inst) override;

private:
    sp<PyInstance> _instance;

};

}
}
}

#endif
