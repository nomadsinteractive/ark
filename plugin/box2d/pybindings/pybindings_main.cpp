#include "core/base/plugin.h"

#include "core/base/api.h"
#include "core/base/plugin_manager.h"
#include "core/types/shared_ptr.h"
#include "core/util/conversions.h"

#include "app/view/arena.h"

#include "python/impl/script/python_script.h"
#include "python/extension/python_interpreter.h"

#include "box2d/impl/body.h"

#include "generated/py_box2d_bindings.h"

using namespace ark;
using namespace ark::plugin::box2d;
using namespace ark::plugin::python;

namespace {

PyMethodDef ARK_BOX2D_METHODS[] = {
    {NULL, NULL, 0, NULL}
};

class Box2dPybindingsPlugin : public Plugin {
public:
    Box2dPybindingsPlugin()
        : Plugin("box2d-pybindings", Plugin::PLUGIN_TYPE_CORE) {
    }

    virtual void createScriptModule(const sp<Script>& script) override {
        const sp<PythonScript> pythonScript = script.as<PythonScript>();
        if(pythonScript) {
            PyObject* arkmodule = pythonScript->arkModule();

            static struct PyModuleDef cModPyArkBox2d = {
                PyModuleDef_HEAD_INIT,
                "box2d",                /* name of module */
                "ark.box2d module",     /* module documentation, may be NULL */
                -1,                     /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
                ARK_BOX2D_METHODS
            };

            PyObject* box2dmodule = PyModule_Create(&cModPyArkBox2d);
            __init_py_box2d_bindings__(box2dmodule);
            PyModule_AddObject(arkmodule, "box2d", box2dmodule);

            PyArkType* type = PythonInterpreter::instance()->getPyArkType<Arena>();
            {
                std::map<TypeId, PyArkType::LoaderFunction>& loader = type->ensureLoader("load");
                loader[Type<World>::id()] = [](PyArkType::Instance& inst, const String& id, const sp<Scope>& args)->Box { return inst.unpack<Arena>()->load<World>(id, args).pack(); };
                loader[Type<Body>::id()] = [](PyArkType::Instance& inst, const String& id, const sp<Scope>& args)->Box { return inst.unpack<Arena>()->load<Body>(id, args).pack(); };
            }
        }
    }
};

}

namespace ark {
namespace plugin {
namespace python {

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::fromType<Body::Type>(const Body::Type& value)
{
    return PyLong_FromLong(static_cast<long>(value));
}

}
}
}

extern "C" ARK_API void __ark_box2d_pybindings_initialize__(Ark&, PluginManager& pm);

void __ark_box2d_pybindings_initialize__(Ark& ark, PluginManager& pm)
{
    Ark::push(ark);
    pm.addPlugin(sp<Box2dPybindingsPlugin>::make());
}
