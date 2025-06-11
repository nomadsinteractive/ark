#include "core/base/plugin.h"

#include "core/base/api.h"
#include "core/base/plugin_manager.h"
#include "core/types/shared_ptr.h"

#include "python/impl/interpreter/python_interpreter.h"
#include "python/extension/python_extension.h"

#include "box2d/impl/rigidbody_box2d.h"

#include "generated/py_box2d_bindings.h"

using namespace ark;
using namespace ark::plugin::box2d;
using namespace ark::plugin::python;

namespace {

class Box2dPybindingsPlugin final : public Plugin {
public:
    Box2dPybindingsPlugin()
        : Plugin("box2d-pybindings", Plugin::PLUGIN_TYPE_CORE) {
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
