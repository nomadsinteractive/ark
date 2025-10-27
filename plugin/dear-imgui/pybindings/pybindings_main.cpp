#include "base/imgui_type.h"
#include "core/base/plugin.h"

#include "core/base/api.h"
#include "core/base/resource_loader.h"
#include "core/types/shared_ptr.h"

#include "generated/py_dear-imgui_bindings.h"

using namespace ark;
using namespace ark::plugin::dear_imgui;
using namespace ark::plugin::python;

namespace {

template<typename T> Box PyArkResourceLoader_loadFunction(PyArkType::Instance& inst, const String& id, const Scope& args) {
    const sp<T> bean = inst.unpack<ResourceLoader>()->load<T>(id, args);
    return Box(bean);
}

PyMethodDef ARK_DEAR_IMGUI_METHODS[] = {
    {nullptr, nullptr, 0, nullptr}
};

class DearImguiPybindingsPlugin final : public Plugin {
public:
    DearImguiPybindingsPlugin()
        : Plugin("dear-imgui-pybindings", Plugin::PLUGIN_TYPE_CORE) {
    }

    void createScriptModule(Interpreter& script) override {
        PythonExtension::instance().addModulePlugin<DearImguiPybindingsPlugin>(*this, script, "dear_imgui", "ark.dear_imgui module", ARK_DEAR_IMGUI_METHODS);
    }

    void initialize(PyObject* dearimguimodule) {
        __init_py_dear_imgui_bindings__(dearimguimodule);
        Map<TypeId, PyArkType::LoaderFunction>& loader = PyArkType::loaders();
        loader[Type<Imgui>::id()] = PyArkResourceLoader_loadFunction<Imgui>;
    }
};

}

extern "C" ARK_API Plugin* __ark_dear_imgui_pybindings_initialize__(Ark&);

Plugin* __ark_dear_imgui_pybindings_initialize__(Ark&)
{
    return new DearImguiPybindingsPlugin();
}
