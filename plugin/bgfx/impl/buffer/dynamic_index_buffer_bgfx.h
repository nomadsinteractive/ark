#pragma once

#include "bgfx/impl/buffer/buffer_base.h"

namespace ark::plugin::bgfx {

class DynamicIndexBufferBgfx : public BufferBase {
public:
    DynamicIndexBufferBgfx();

    uint64_t id() override;
    ResourceRecycleFunc recycle() override;
    void upload(GraphicsContext& graphicsContext) override;
    void uploadBuffer(GraphicsContext& graphicsContext, Uploader& input) override;
    void downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr) override;

    void bind() override;

private:
    DynamicIndexBuffer _handle;
    std::vector<uint8_t> _indices;
};

}
