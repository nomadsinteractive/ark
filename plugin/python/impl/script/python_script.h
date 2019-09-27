#ifndef ARK_PLUGIN_PYTHON_IMPL_SCRIPT_INTERPRETER_PYTHON_SCRIPT_H_
#define ARK_PLUGIN_PYTHON_IMPL_SCRIPT_INTERPRETER_PYTHON_SCRIPT_H_

#include <Python.h>

#include "core/ark.h"
#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/script.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "python/api.h"

namespace ark {
namespace plugin {
namespace python {

class PythonInterpreter;

class ARK_PLUGIN_PYTHON_API PythonScript : public Script, public Implements<PythonScript, Script> {
public:
    PythonScript(const String& name, const document& libraries);
    ~PythonScript();

    virtual void run(const sp<Asset>& script, const Scope& vars) override;
    virtual Box call(const String& function, const Arguments& args) override;

    PyObject* arkModule();

//  [[plugin::builder("python")]]
    class BUILDER : public Builder<Script> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Script> build(const Scope& args) override;

    private:
        document _manifest;
    };

private:
    void addScopeToDict(PyObject* dict, const Scope& scope);
    PyObject* argumentsToTuple(const Arguments& args);

private:
    std::wstring _name;
    PyObject* _ark_module;
};

}
}
}

#endif
