#include "sdl3/api.h"

#include "impl/renderer_factory/renderer_factory_sdl3_gpu.h"

namespace ark::plugin::sdl3 {

sp<RendererFactory> make_renderer_factory_sdl3_gpu()
{
    return sp<RendererFactory>::make<RendererFactorySDL3_GPU>();
}

}
