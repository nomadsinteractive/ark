#include "core/base/plugin.h"

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "python/extension/python_interpreter.h"
#include "generated/py_dear-imgui_bindings.h"

using namespace ark;
using namespace ark::plugin::dear_imgui;
using namespace ark::plugin::python;

namespace {

PyMethodDef ARK_DEAR_IMGUI_METHODS[] = {
    {nullptr, nullptr, 0, nullptr}
};

class DearImguiPybindingsPlugin : public Plugin {
public:
    DearImguiPybindingsPlugin()
        : Plugin("dear-imgui-pybindings", Plugin::PLUGIN_TYPE_CORE) {
    }

    virtual void createScriptModule(const sp<Script>& script) override {
        PythonInterpreter::instance()->addModulePlugin<DearImguiPybindingsPlugin>(*this, script, "dear_imgui", "ark.dear_imgui module", ARK_DEAR_IMGUI_METHODS);
    }

    void initialize(PyObject* dearimguimodule) {
        __init_py_dear_imgui_bindings__(dearimguimodule);
    }
};

}

extern "C" ARK_API Plugin* __ark_dear_imgui_pybindings_initialize__(Ark&);

Plugin* __ark_dear_imgui_pybindings_initialize__(Ark&)
{
    return new DearImguiPybindingsPlugin();
}
