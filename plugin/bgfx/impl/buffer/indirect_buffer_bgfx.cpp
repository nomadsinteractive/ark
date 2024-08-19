#include "bgfx/impl/buffer/indirect_buffer_bgfx.h"

#include "core/util/uploader_type.h"

namespace ark::plugin::bgfx {

namespace {

struct DrawElementsIndirectCommandBgfx {
    DrawingParams::DrawElementsIndirectCommand _indirect_command;
    uint32_t _paddings[3] = {0};
};

}


IndirectBufferBgfx::IndirectBufferBgfx()
    : BufferBase(Buffer::TYPE_DRAW_INDIRECT, Buffer::USAGE_BIT_DYNAMIC)
{
}

uint64_t IndirectBufferBgfx::id()
{
    return _handle.id();
}

ResourceRecycleFunc IndirectBufferBgfx::recycle()
{
    return _handle.recycle();
}

void IndirectBufferBgfx::upload(GraphicsContext& graphicsContext)
{
    const size_t commandCount = _indirect_commands.size() / sizeof(DrawingParams::DrawElementsIndirectCommand);
    const ::bgfx::Memory* memory = ::bgfx::alloc(commandCount * sizeof(DrawElementsIndirectCommandBgfx));
    const DrawingParams::DrawElementsIndirectCommand* indirectCommandSrc = reinterpret_cast<const DrawingParams::DrawElementsIndirectCommand*>(_indirect_commands.data());
    DrawElementsIndirectCommandBgfx* indirectCommandDst = reinterpret_cast<DrawElementsIndirectCommandBgfx*>(memory->data);
    for(size_t i = 0; i < commandCount; ++i)
        indirectCommandDst[i] = {indirectCommandSrc[i]};

    _handle.reset(::bgfx::createIndirectBuffer(commandCount));
    // ::bgfx::DynamicVertexBufferHandle vertexHandle = {static_cast<uint16_t>(_handle.id() - 1)};
    // ::bgfx::update(vertexHandle, 0, memory);
}

void IndirectBufferBgfx::uploadBuffer(GraphicsContext& graphicsContext, Uploader& input)
{
    _indirect_commands = UploaderType::toBytes(input);
    upload(graphicsContext);
}

void IndirectBufferBgfx::downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr)
{
    FATAL("Unimplemented");
}

void IndirectBufferBgfx::bind()
{
    // ::bgfx::setBuffer(0, _handle, ::bgfx::Access::Read);
}

void IndirectBufferBgfx::bindRange(uint32_t first, uint32_t count)
{
}

const IndirectBuffer& IndirectBufferBgfx::handle() const
{
    return _handle;
}

}
