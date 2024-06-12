#include "core/base/plugin.h"

#include "core/base/api.h"
#include "core/base/plugin_manager.h"
#include "core/types/shared_ptr.h"
#include "core/util/string_convert.h"

#include "app/view/arena.h"

#include "python/impl/script/python_script.h"
#include "python/extension/python_interpreter.h"

#include "generated/py_bullet_bindings.h"

#include "plugin/bullet/forwarding.h"

using namespace ark;
using namespace ark::plugin::bullet;
using namespace ark::plugin::python;

namespace {

PyMethodDef ARK_BULLET_METHODS[] = {
    {NULL, NULL, 0, NULL}
};

class BulletPybindingsPlugin : public Plugin {
public:
    BulletPybindingsPlugin()
        : Plugin("bullet-pybindings", Plugin::PLUGIN_TYPE_CORE) {
    }

    void createScriptModule(Interpreter& script) override {
        PythonInterpreter::instance().addModulePlugin<BulletPybindingsPlugin>(*this, script, "bullet", "ark.bullet module", ARK_BULLET_METHODS);

        PyArkType* pyResourceLoaderType = PythonInterpreter::instance().getPyArkType<ResourceLoader>();
        {
            std::map<TypeId, PyArkType::LoaderFunction>& loader = pyResourceLoaderType->ensureLoader("load");
            loader[Type<ColliderBullet>::id()] = [](PyArkType::Instance& inst, const String& id, const Scope& args)->Box { return inst.unpack<ResourceLoader>()->load<ColliderBullet>(id, args); };
            loader[Type<RigidBodyBullet>::id()] = [](PyArkType::Instance& inst, const String& id, const Scope& args)->Box { return inst.unpack<ResourceLoader>()->load<RigidBodyBullet>(id, args); };
        }
        PyArkType* pyArenaType = PythonInterpreter::instance().getPyArkType<Arena>();
        {
            std::map<TypeId, PyArkType::LoaderFunction>& loader = pyArenaType->ensureLoader("load");
            loader[Type<ColliderBullet>::id()] = [](PyArkType::Instance& inst, const String& id, const Scope& args)->Box { return inst.unpack<Arena>()->load<ColliderBullet>(id, args); };
            loader[Type<RigidBodyBullet>::id()] = [](PyArkType::Instance& inst, const String& id, const Scope& args)->Box { return inst.unpack<Arena>()->load<RigidBodyBullet>(id, args); };
        }
    }

    void initialize(PyObject* bulletmodule) {
        __init_py_bullet_bindings__(bulletmodule);
    }
};

}

extern "C" ARK_API Plugin* __ark_bullet_pybindings_initialize__(Ark&);

Plugin* __ark_bullet_pybindings_initialize__(Ark& /*ark*/)
{
    return new BulletPybindingsPlugin();
}
