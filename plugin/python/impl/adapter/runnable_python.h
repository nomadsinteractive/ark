#pragma once

#include "core/inf/debris.h"
#include "core/inf/runnable.h"
#include "core/types/implements.h"

#include "python/extension/py_instance.h"

namespace ark::plugin::python {

class RunnablePython final : public Runnable, public Debris, Implements<RunnablePython, Runnable, Debris> {
public:
    RunnablePython(PyInstance callable);

    void run() override;

    void traverse(const Visitor& visitor) override;

private:
    PyInstance _callable;
};

}
