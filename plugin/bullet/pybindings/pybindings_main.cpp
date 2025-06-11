#include "core/base/plugin.h"

#include "core/base/api.h"
#include "core/base/plugin_manager.h"
#include "core/types/shared_ptr.h"

#include "python/impl/interpreter/python_interpreter.h"
#include "python/extension/python_extension.h"

#include "generated/py_bullet_bindings.h"

#include "plugin/bullet/forwarding.h"

using namespace ark;
using namespace ark::plugin::bullet;
using namespace ark::plugin::python;

namespace {

class BulletPybindingsPlugin final : public Plugin {
public:
    BulletPybindingsPlugin()
        : Plugin("bullet-pybindings", Plugin::PLUGIN_TYPE_CORE) {
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
