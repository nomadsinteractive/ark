#include "sdl3-gpu/impl/renderer_factory/renderer_factory_sdl3_gpu.h"

#include "sdl3/api.h"

namespace ark::plugin::sdl3_gpu {

sp<RendererFactory> RendererFactorySDL3_GPU::BUILDER::build(const Scope& args)
{
    return sdl3::make_renderer_factory_sdl3_gpu();
}

}
