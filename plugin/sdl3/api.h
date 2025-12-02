#pragma  once

#include "core/base/api.h"

#include "renderer/forwarding.h"

#ifndef ARK_BUILD_STATIC_PLUGINS
    #ifdef ARK_FLAG_BUILDING_PLUGIN_SDL3
    #   define ARK_PLUGIN_SDL3_API ARK_API_EXPORT_DECLARATION
    #else
    #   define ARK_PLUGIN_SDL3_API ARK_API_IMPORT_DECLARATION
    #endif
#else
#   define ARK_PLUGIN_SDL3_API
#endif


namespace ark::plugin::sdl3 {

ARK_PLUGIN_SDL3_API sp<RendererFactory> make_renderer_factory_sdl3_gpu();

}