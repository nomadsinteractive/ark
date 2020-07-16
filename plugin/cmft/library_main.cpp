#include "core/base/plugin.h"

#include "core/base/api.h"
#include "core/base/plugin_manager.h"
#include "core/types/shared_ptr.h"

#include "generated/cmft_plugin.h"

using namespace ark;

extern "C" ARK_API Plugin* __ark_cmft_initialize__(Ark&);

Plugin* __ark_cmft_initialize__(Ark&)
{
    return new ark::plugin::cmft::CmftPlugin();
}
