#pragma once

#include "bgfx/impl/buffer/buffer_base.h"

namespace ark::plugin::bgfx {

class StorageBufferBgfx final : public BufferBase {
public:
    StorageBufferBgfx();
    
    uint64_t id() override;
    ResourceRecycleFunc recycle() override;
    
    void upload(GraphicsContext& graphicsContext) override;
    void uploadBuffer(GraphicsContext& graphicsContext, Uploader& input) override;

    void downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr) override;

    void bind() override;
    void bindRange(uint32_t first, uint32_t count) override;

private:
    DynamicIndexBuffer _handle;
    std::vector<uint8_t> _data;
};

}
