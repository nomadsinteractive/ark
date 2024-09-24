#pragma once

#include "core/inf/runnable.h"
#include "core/types/implements.h"

#include "python/forwarding.h"
#include "python/extension/py_instance.h"

namespace ark::plugin::python {

class PythonCallableRunnable final : public Runnable, Implements<PythonCallableRunnable, Runnable> {
public:
    PythonCallableRunnable(PyInstance callable);

    virtual void run() override;

private:
    PyInstance _callable;
};

}
