#ifndef ARK_PLUGIN_DEAR_IMGUI_API_H_
#define ARK_PLUGIN_DEAR_IMGUI_API_H_

#include "core/base/api.h"

#ifndef ARK_BUILD_STATIC_PLUGINS
    #ifdef ARK_FLAG_BUILDING_PLUGIN_DEAR_IMGUI
    #   define ARK_PLUGIN_DEAR_IMGUI_API ARK_API_EXPORT_DECLARATION
    #else
    #   define ARK_PLUGIN_DEAR_IMGUI_API ARK_API_IMPORT_DECLARATION
    #endif
#else
#   define ARK_PLUGIN_DEAR_IMGUI_API
#endif

#endif
