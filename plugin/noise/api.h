#pragma once

#include "core/base/api.h"

#ifndef ARK_BUILD_STATIC_PLUGINS
    #ifdef ARK_FLAG_BUILDING_PLUGIN_NOISE
    #   define ARK_PLUGIN_NOISE_API ARK_API_EXPORT_DECLARATION
    #else
    #   define ARK_PLUGIN_NOISE_API ARK_API_IMPORT_DECLARATION
    #endif
#else
#   define ARK_PLUGIN_NOISE_API
#endif
