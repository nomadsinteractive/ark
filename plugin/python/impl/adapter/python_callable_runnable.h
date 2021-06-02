#ifndef ARK_PLUGIN_PYTHON_IMPL_RUNNABLE_PYTHON_CALLABLE_RUNNABLE_H_
#define ARK_PLUGIN_PYTHON_IMPL_RUNNABLE_PYTHON_CALLABLE_RUNNABLE_H_

#include "core/inf/holder.h"
#include "core/inf/runnable.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "python/forwarding.h"
#include "python/extension/py_instance.h"

namespace ark {
namespace plugin {
namespace python {

class PythonCallableRunnable : public Runnable, public Holder, Implements<PythonCallableRunnable, Runnable, Holder> {
public:
    PythonCallableRunnable(PyInstance callable);

    virtual void run() override;

    virtual void traverse(const Visitor& visitor) override;

private:
    PyInstance _callable;
};

}
}
}

#endif
