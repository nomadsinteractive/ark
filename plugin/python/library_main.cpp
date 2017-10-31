#include "core/base/plugin.h"

#include "core/ark.h"
#include "core/base/api.h"
#include "core/base/plugin_manager.h"
#include "core/types/shared_ptr.h"

#include "generated/python_plugin.h"

using namespace ark;
using namespace ark::plugin::python;

extern "C" ARK_API Plugin* __ark_python_initialize__(Ark&);

Plugin* __ark_python_initialize__(Ark& ark)
{
    ark.push();
    return new PythonPlugin();
}
