#ifndef ARK_PLUGIN_PYTHON_IMPL_DUCK_PY_OBJECT_DUCK_TYPE_H_
#define ARK_PLUGIN_PYTHON_IMPL_DUCK_PY_OBJECT_DUCK_TYPE_H_

#include "core/inf/duck.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

#include "python/forwarding.h"
#include "python/extension/py_instance.h"

namespace ark {
namespace plugin {
namespace python {

class PyObjectDuckType : public Duck<CollisionCallback>, public Duck<RendererMaker>, public Duck<Array<Color>>, public Duck<Array<int32_t>>,
        public Implements<PyObjectDuckType, Duck<CollisionCallback>, Duck<RendererMaker>, Duck<Array<Color>>, Duck<Array<int32_t>>> {
public:
    PyObjectDuckType(PyInstance inst);

    virtual void to(sp<CollisionCallback>& inst) override;
    virtual void to(sp<RendererMaker>& inst) override;
    virtual void to(sp<Array<Color>>& inst) override;
    virtual void to(sp<Array<int32_t>>& inst) override;

private:
    PyInstance _instance;

};

}
}
}

#endif
