#pragma once

#include "core/inf/duck.h"
#include "core/types/implements.h"

#include "graphics/forwarding.h"

#include "python/extension/py_instance.h"

namespace ark::plugin::python {

class PyListDuckType : public Duck<std::vector<Box>>, public Duck<Vec2>, public Duck<Vec3>, public Duck<Vec4>, Implements<PyListDuckType, Duck<std::vector<Box>>, Duck<Vec2>, Duck<Vec3>, Duck<Vec4>> {
public:
    PyListDuckType(PyInstance instance);

    void to(sp<Vec2>& inst) override;
    void to(sp<Vec3>& inst) override;
    void to(sp<Vec4>& inst) override;
    void to(sp<Vector<Box>>& inst) override;

private:
    PyInstance _instance;
};

} // python
