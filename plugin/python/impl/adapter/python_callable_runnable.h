#ifndef ARK_PLUGIN_PYTHON_IMPL_RUNNABLE_PYTHON_CALLABLE_RUNNABLE_H_
#define ARK_PLUGIN_PYTHON_IMPL_RUNNABLE_PYTHON_CALLABLE_RUNNABLE_H_

#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/epi/disposed.h"
#include "core/types/class.h"
#include "core/types/shared_ptr.h"

#include "python/extension/py_instance.h"

namespace ark {
namespace plugin {
namespace python {

class PythonCallableRunnable : public Runnable, public Disposed, Implements<PythonCallableRunnable, Runnable, Disposed> {
public:
    PythonCallableRunnable(const sp<PyInstance>& callable);

    virtual void run() override;

private:

    enum FunctionResult {
        FUNCTION_RESULT_NONE,
        FUNCTION_RESULT_TRUE,
        FUNCTION_RESULT_FALSE
    };

    class Result : public Boolean {
    public:
        Result();

        virtual bool val() override;

        void setFunctionResult(FunctionResult functionResult);

    private:
        FunctionResult _function_result;
    };

private:
    PyInstance _args;
    sp<PyInstance> _callable;
    sp<Result> _result;

};

}
}
}

#endif
