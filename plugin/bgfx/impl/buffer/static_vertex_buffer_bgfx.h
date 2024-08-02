#pragma once

#include "bgfx/impl/buffer/buffer_base.h"

namespace ark::plugin::bgfx {

class StaticVertexBufferBgfx final : public BufferBase {
public:
    StaticVertexBufferBgfx();

    uint64_t id() override;
    ResourceRecycleFunc recycle() override;
    void setupLayout(const PipelineDescriptor& pipelineDescriptor) override;
    void upload(GraphicsContext& graphicsContext) override;
    void uploadBuffer(GraphicsContext& graphicsContext, Uploader& input) override;
    void downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr) override;

    void bind() override;

private:
    StaticVertexBuffer _handle;
    std::vector<uint8_t> _data;
    ::bgfx::VertexLayout _vertex_buffer_layout;
};

}