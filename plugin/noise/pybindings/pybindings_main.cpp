#include "core/base/plugin.h"

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "python/extension/python_interpreter.h"

#include "generated/py_noise_bindings.h"

using namespace ark;
using namespace ark::plugin::noise;
using namespace ark::plugin::python;

namespace {

PyMethodDef ARK_NOISE_METHODS[] = {
    {NULL, NULL, 0, NULL}
};

class NoisePybindingsPlugin : public Plugin {
public:
    NoisePybindingsPlugin()
        : Plugin("noise-pybindings", Plugin::PLUGIN_TYPE_CORE) {
    }

    void createScriptModule(Interpreter& script) override {
        PythonInterpreter::instance().addModulePlugin<NoisePybindingsPlugin>(*this, script, "noise", "ark.noise module", ARK_NOISE_METHODS);
    }

    void initialize(PyObject* noisemodule) {
        __init_py_noise_bindings__(noisemodule);
    }
};

}

extern "C" ARK_API Plugin* __ark_noise_pybindings_initialize__(Ark&);

Plugin* __ark_noise_pybindings_initialize__(Ark&)
{
    return new NoisePybindingsPlugin();
}
