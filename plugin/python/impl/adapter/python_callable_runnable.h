#ifndef ARK_PLUGIN_PYTHON_IMPL_RUNNABLE_PYTHON_CALLABLE_RUNNABLE_H_
#define ARK_PLUGIN_PYTHON_IMPL_RUNNABLE_PYTHON_CALLABLE_RUNNABLE_H_

#include "core/inf/holder.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/epi/disposed.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "python/forwarding.h"
#include "python/extension/py_instance.h"

namespace ark {
namespace plugin {
namespace python {

class PythonCallableRunnable : public Runnable, public Disposed, public Holder, Implements<PythonCallableRunnable, Runnable, Disposed, Holder> {
public:
    PythonCallableRunnable(PyInstance callable);

    virtual void run() override;

    virtual void traverse(const Visitor& visitor) override;

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
    PyInstance _callable;
    sp<Result> _result;

};

}
}
}

#endif
