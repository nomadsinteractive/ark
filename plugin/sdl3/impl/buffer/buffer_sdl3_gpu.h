#pragma once

#include <SDL3/SDL.h>

#include "renderer/base/buffer.h"
#include "sdl3/base/context_sdl3_gpu.h"

namespace ark::plugin::sdl3 {

class Buffer_SDL3_GPU final : public Buffer::Delegate {
public:
    Buffer_SDL3_GPU(SDL_GPUBufferUsageFlags usageFlags);

    uint64_t id() override;

    void upload(GraphicsContext& graphicsContext) override;
    ResourceRecycleFunc recycle() override;

    void uploadBuffer(GraphicsContext& graphicsContext, Uploader& input) override;
    void downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr) override;

    SDL_GPUBuffer* buffer() const;

private:
    SDL_GPUBufferUsageFlags _usage_flags;
    SDL_GPUBuffer* _buffer;
};

}
