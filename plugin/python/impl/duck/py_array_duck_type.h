#ifndef ARK_PLUGIN_PYTHON_IMPL_DUCK_PY_ARRAY_DUCK_TYPE_H_
#define ARK_PLUGIN_PYTHON_IMPL_DUCK_PY_ARRAY_DUCK_TYPE_H_

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

class PyArrayDuckType : public Duck<Array<sp<Mat3>>>, public Duck<Array<sp<Mat4>>>, public Duck<Array<sp<Input>>>,
        public Implements<PyArrayDuckType, Duck<Array<sp<Mat3>>>, Duck<Array<sp<Mat4>>>, Duck<Array<sp<Input>>>> {
public:
[[deprecated]]
    PyArrayDuckType(PyInstance inst);

    virtual void to(sp<Array<sp<Mat3>>>& inst) override;
    virtual void to(sp<Array<sp<Mat4>>>& inst) override;
    virtual void to(sp<Array<sp<Input>>>& inst) override;

private:
    PyInstance _instance;
};

}
}
}

#endif
