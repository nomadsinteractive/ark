#include "core/base/plugin.h"

#include "core/base/api.h"
#include "core/base/plugin_manager.h"
#include "core/types/shared_ptr.h"
#include "core/util/string_convert.h"

#include "python/impl/script/python_script.h"
#include "generated/py_dear-imgui_bindings.h"

using namespace ark;
using namespace ark::plugin::dear_imgui;
using namespace ark::plugin::python;

namespace {

PyMethodDef ARK_DEAR_IMGUI_METHODS[] = {
    {nullptr, nullptr, 0, nullptr}
};

class Box2dPybindingsPlugin : public Plugin {
public:
    Box2dPybindingsPlugin()
        : Plugin("dear-imgui-pybindings", Plugin::PLUGIN_TYPE_CORE) {
    }

    virtual void createScriptModule(const sp<Script>& script) override {
        const sp<PythonScript> pythonScript = script.as<PythonScript>();
        if(pythonScript) {
            PyObject* arkmodule = pythonScript->arkModule();

            static struct PyModuleDef cModPyArkDearImgui = {
                PyModuleDef_HEAD_INIT,
                "dear_imgui",                /* name of module */
                "ark.dear_imgui module",     /* module documentation, may be NULL */
                -1,                     /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
                ARK_DEAR_IMGUI_METHODS
            };

            PyObject* dearimguimodule = PyModule_Create(&cModPyArkDearImgui);
            __init_py_dear_imgui_bindings__(dearimguimodule);
            PyModule_AddObject(arkmodule, "dear_imgui", dearimguimodule);

        }
    }
};

}

extern "C" ARK_API Plugin* __ark_dear_imgui_pybindings_initialize__(Ark&);

Plugin* __ark_dear_imgui_pybindings_initialize__(Ark&)
{
    return new Box2dPybindingsPlugin();
}
