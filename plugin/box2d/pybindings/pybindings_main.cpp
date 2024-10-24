#include "core/base/plugin.h"

#include "core/base/api.h"
#include "core/base/plugin_manager.h"
#include "core/types/shared_ptr.h"
#include "core/util/string_convert.h"

#include "app/view/arena.h"

#include "python/impl/interpreter/python_interpreter.h"
#include "python/extension/python_extension.h"

#include "box2d/impl/rigid_body_box2d.h"

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

    void createScriptModule(Interpreter& script) override {
        PythonExtension::instance().addModulePlugin<Box2dPybindingsPlugin>(*this, script, "box2d", "ark.box2d module", ARK_BOX2D_METHODS);

        PyArkType* pyResourceLoaderType = PythonExtension::instance().getPyArkType<ResourceLoader>();
        {
            std::map<TypeId, PyArkType::LoaderFunction>& loader = pyResourceLoaderType->ensureLoader("load");
            loader[Type<ColliderBox2D>::id()] = [](PyArkType::Instance& inst, const String& id, const Scope& args)->Box { return inst.unpack<ResourceLoader>()->load<ColliderBox2D>(id, args); };
            loader[Type<RigidBodyBox2D>::id()] = [](PyArkType::Instance& inst, const String& id, const Scope& args)->Box { return inst.unpack<ResourceLoader>()->load<RigidBodyBox2D>(id, args); };
        }
        PyArkType* pyArenaType = PythonExtension::instance().getPyArkType<Arena>();
        {
            std::map<TypeId, PyArkType::LoaderFunction>& loader = pyArenaType->ensureLoader("load");
            loader[Type<ColliderBox2D>::id()] = [](PyArkType::Instance& inst, const String& id, const Scope& args)->Box { return inst.unpack<Arena>()->load<ColliderBox2D>(id, args); };
            loader[Type<RigidBodyBox2D>::id()] = [](PyArkType::Instance& inst, const String& id, const Scope& args)->Box { return inst.unpack<Arena>()->load<RigidBodyBox2D>(id, args); };
        }
    }

    void initialize(PyObject* module) {
        __init_py_box2d_bindings__(module);
    }
};

}

extern "C" ARK_API Plugin* __ark_box2d_pybindings_initialize__(Ark&);

Plugin* __ark_box2d_pybindings_initialize__(Ark& ark)
{
    return new Box2dPybindingsPlugin();
}
