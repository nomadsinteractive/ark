#pragma once

#include <vector>

#include "core/inf/duck.h"
#include "core/types/implements.h"

#include "python/extension/py_instance.h"

namespace ark::plugin::python {

class PyListDuckType : public Duck<std::vector<Box>>, Implements<PyListDuckType, Duck<std::vector<Box>>> {
public:
    PyListDuckType(PyInstance instance);

    void to(sp<std::vector<Box>>& inst) override;

private:
    PyInstance _instance;
};

} // python
