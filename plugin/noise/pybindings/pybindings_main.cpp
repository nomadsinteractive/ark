#include "core/base/plugin.h"

#include "core/base/api.h"
#include "core/base/plugin_manager.h"
#include "core/types/shared_ptr.h"
#include "core/util/string_convert.h"

#include "python/impl/script/python_script.h"
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

    virtual void createScriptModule(const sp<Script>& script) override {
        const sp<PythonScript> pythonScript = script.as<PythonScript>();
        if(pythonScript) {
            PyObject* arkmodule = pythonScript->arkModule();

            static struct PyModuleDef cModPyArkNoise = {
                PyModuleDef_HEAD_INIT,
                "noise",                /* name of module */
                "ark.noise module",     /* module documentation, may be NULL */
                -1,                     /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
                ARK_NOISE_METHODS
            };

            PyObject* noisemodule = PyModule_Create(&cModPyArkNoise);
            __init_py_noise_bindings__(noisemodule);
            PyModule_AddObject(arkmodule, "noise", noisemodule);

        }
    }
};

}

extern "C" ARK_API Plugin* __ark_noise_pybindings_initialize__(Ark&);

Plugin* __ark_noise_pybindings_initialize__(Ark&)
{
    return new NoisePybindingsPlugin();
}
