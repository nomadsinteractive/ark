#pragma once

#include "bgfx/impl/buffer/buffer_base.h"

namespace ark::plugin::bgfx {

class DynamicVertexBufferBgfx final : public BufferBase {
public:
    DynamicVertexBufferBgfx();

    uint64_t id() override;
    ResourceRecycleFunc recycle() override;
    void upload(GraphicsContext& graphicsContext) override;
    void uploadBuffer(GraphicsContext& graphicsContext, Uploader& input) override;
    void downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr) override;

    void setupLayout(const PipelineDescriptor& pipelineDescriptor);
    void bind() override;
    void bindRange(uint32_t first, uint32_t count) override;

private:
    DynamicVertexBuffer _handle;
    std::vector<uint8_t> _data;
    ::bgfx::VertexLayout _vertex_buffer_layout;
};

}