#pragma once

#include <SDL3/SDL.h>

#include "renderer/base/buffer.h"

namespace ark::plugin::sdl3 {

class BufferSDL3_GPU final : public Buffer::Delegate {
public:
    BufferSDL3_GPU(const SDL_GPUBufferUsageFlags usageFlags);

    uint64_t id() override;
    void upload(GraphicsContext& graphicsContext) override;
    ResourceRecycleFunc recycle() override;
    void uploadBuffer(GraphicsContext& graphicsContext, Uploader& input) override;
    void downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr) override;

private:
    SDL_GPUBufferUsageFlags _usage_flags;
    SDL_GPUBuffer* _buffer;
    uint32_t _buffer_size;
};

}
