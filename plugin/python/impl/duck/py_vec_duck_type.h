#ifndef ARK_PLUGIN_PYTHON_IMPL_DUCK_PY_VEC_DUCK_TYPE_H_
#define ARK_PLUGIN_PYTHON_IMPL_DUCK_PY_VEC_DUCK_TYPE_H_

#include "core/forwarding.h"
#include "core/inf/duck.h"
#include "core/types/implements.h"

#include "graphics/forwarding.h"

#include "python/forwarding.h"
#include "python/extension/py_instance.h"

namespace ark {
namespace plugin {
namespace python {

class [[deprecated]] PyVecDuckType : public Duck<Vec2>, public Duck<Vec3>, public Duck<Size>, public Duck<Vec4>, public Duck<Rotation>, public Implements<PyVecDuckType, Duck<Vec2>, Duck<Vec3>, Duck<Size>, Duck<Vec4>, Duck<Rotation>> {
public:
    PyVecDuckType(PyInstance inst);

    virtual void to(sp<Vec2>& inst) override;
    virtual void to(sp<Vec3>& inst) override;
    virtual void to(sp<Size>& inst) override;
    virtual void to(sp<Vec4>& inst) override;
    virtual void to(sp<Rotation>& inst) override;

private:
    PyInstance _instance;

};

}
}
}

#endif
