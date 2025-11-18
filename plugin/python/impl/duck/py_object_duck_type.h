#pragma once

#include "core/inf/duck.h"
#include "core/types/implements.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

#include "python/forwarding.h"
#include "python/extension/py_instance.h"

namespace ark::plugin::python {

class PyObjectDuckType final : public Duck<StringVar>, public Duck<String>, public Duck<Integer>, public Duck<Numeric>, public Duck<Box>, Implements<PyObjectDuckType, Duck<StringVar>, Duck<String>, Duck<Integer>, Duck<Numeric>, Duck<Box>> {
public:
    PyObjectDuckType(PyInstance inst);

    void to(sp<StringVar>& inst) override;
    void to(sp<String>& inst) override;
    void to(sp<Integer>& inst) override;
    void to(sp<Numeric>& inst) override;
    void to(sp<Box>& inst) override;

private:
    PyInstance _instance;

};

}
