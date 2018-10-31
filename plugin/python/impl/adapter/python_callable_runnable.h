#ifndef ARK_PLUGIN_PYTHON_IMPL_RUNNABLE_PYTHON_CALLABLE_RUNNABLE_H_
#define ARK_PLUGIN_PYTHON_IMPL_RUNNABLE_PYTHON_CALLABLE_RUNNABLE_H_

#include "core/inf/runnable.h"
#include "core/epi/lifecycle.h"
#include "core/types/class.h"
#include "core/types/shared_ptr.h"

#include "python/extension/py_instance.h"

namespace ark {
namespace plugin {
namespace python {

class PythonCallableRunnable : public Runnable, public Lifecycle, Implements<PythonCallableRunnable, Runnable, Lifecycle> {
public:
    PythonCallableRunnable(const sp<PyInstance>& callable);

    virtual void run() override;

private:
    PyInstance _args;
    sp<PyInstance> _callable;

};

}
}
}

#endif
