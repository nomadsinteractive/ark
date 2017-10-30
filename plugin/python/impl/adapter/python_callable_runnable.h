#ifndef ARK_PLUGIN_PYTHON_IMPL_RUNNABLE_PYTHON_CALLABLE_RUNNABLE_H_
#define ARK_PLUGIN_PYTHON_IMPL_RUNNABLE_PYTHON_CALLABLE_RUNNABLE_H_

#include "core/inf/runnable.h"
#include "core/epi/expired.h"
#include "core/types/class.h"
#include "core/types/shared_ptr.h"

#include "python/extension/py_instance.h"

namespace ark {
namespace plugin {
namespace python {

class PythonCallableRunnable : public Runnable, public Expired, Implements<PythonCallableRunnable, Runnable, Expired> {
public:
    PythonCallableRunnable(const sp<PyInstance>& callable);

    virtual void run() override;

    virtual bool val() override;

private:
    PyInstance _args;
    sp<PyInstance> _callable;
    bool _not_none_returned;

};

}
}
}

#endif
