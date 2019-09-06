#include "core/base/plugin.h"

#include "core/ark.h"
#include "core/base/api.h"
#include "core/base/plugin_manager.h"
#include "core/types/shared_ptr.h"

#include "generated/dear_imgui_plugin.h"

using namespace ark;
using namespace ark::plugin::dear_imgui;

extern "C" ARK_API Plugin* __ark_dear_imgui_initialize__(Ark&);
extern void __ark_bootstrap_shaders_string_table__();

Plugin* __ark_dear_imgui_initialize__(Ark& ark)
{
    __ark_bootstrap_shaders_string_table__();
    return new DearImguiPlugin();
}
