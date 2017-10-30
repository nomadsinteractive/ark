#ifndef ARK_PLUGIN_BOX2D_API_H_
#define ARK_PLUGIN_BOX2D_API_H_

#include "core/base/api.h"

#ifndef ARK_FLAG_BUILD_SHARED_PLUGIN
    #ifdef ARK_FLAG_BUILDING_PLUGIN_BOX2D
    #   define ARK_PLUGIN_BOX2D_API ARK_API_EXPORT_DECLARATION
    #else
    #   define ARK_PLUGIN_BOX2D_API ARK_API_IMPORT_DECLARATION
    #endif
#else
#   define ARK_PLUGIN_BOX2D_API
#endif

#endif
