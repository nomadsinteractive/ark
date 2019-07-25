#ifndef ARK_PLUGIN_BOX2D_API_H_
#define ARK_PLUGIN_BOX2D_API_H_

#include "core/base/api.h"

#ifndef ARK_BUILD_STATIC_PLUGINS
    #ifdef ARK_FLAG_BUILDING_PLUGIN_BOX2D
    #   define ARK_PLUGIN_BOX2D_API ARK_API_EXPORT_DECLARATION
    #else
    #   define ARK_PLUGIN_BOX2D_API ARK_API_IMPORT_DECLARATION
    #endif
#else
#   define ARK_PLUGIN_BOX2D_API
#endif

#endif
