#pragma once

#include <vector>

#include <Python.h>

#include "core/ark.h"
#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/interpreter.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "python/api.h"

namespace ark::plugin::python {

class ARK_PLUGIN_PYTHON_API PythonInterpreter final : public Interpreter, public Implements<PythonInterpreter, Interpreter> {
public:
    PythonInterpreter(const String& name, const document& libraries);
    ~PythonInterpreter() override;

    void initialize() override;

    void execute(const sp<Asset>& source, const Scope& vars) override;
    Box call(const Box& func, const Arguments& args) override;
    Box attr(const Box& obj, const String& name) override;

    PyObject* arkModule();
    const std::vector<String>& paths() const;

//  [[plugin::builder("python")]]
    class BUILDER final : public Builder<Interpreter> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Interpreter> build(const Scope& args) override;

    private:
        document _manifest;
    };

private:
    std::wstring _name;
    std::vector<String> _paths;
    PyObject* _ark_module;
};

}
