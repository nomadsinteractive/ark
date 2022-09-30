#include "renderer/base/shared_buffer.h"

#include "core/impl/writable/writable_memory.h"

#include "renderer/base/render_controller.h"

namespace ark {

SharedBuffer::SharedBuffer::SharedBuffer(const Buffer& buffer, Uploader::MakerFunc maker, std::function<size_t(size_t)> sizeCalculator)
    : _buffer(buffer), _maker(std::move(maker)), _size_calculator(std::move(sizeCalculator)), _primitive_count(0)
{
}

Buffer::Snapshot SharedBuffer::SharedBuffer::snapshot(RenderController& renderController, size_t primitiveCount, size_t reservedIfInsufficient)
{
    const size_t warningLimit = 20000;
    DWARN(primitiveCount < warningLimit, "Object count(%d) exceeding warning limit(%d). You can make the limit larger if you're sure what you're doing", primitiveCount, warningLimit);
    size_t size = _size_calculator(primitiveCount);
    if(_primitive_count < primitiveCount)
    {
        _primitive_count = primitiveCount + reservedIfInsufficient;
        sp<Uploader> uploader = _maker(_primitive_count);
        DCHECK(uploader && uploader->size() >= size, "Making Uploader failed, primitive-count: %d, uploader-size: %d, required-size: %d", _primitive_count, uploader ? uploader->size() : 0, size);
        renderController.uploadBuffer(_buffer, std::move(uploader), RenderController::US_RELOAD);
    }
    return _buffer.snapshot(size);
}

Uploader::MakerFunc SharedBuffer::Quads::maker()
{
    return [](size_t objectCount)->sp<Uploader> { return sp<Concat>::make(objectCount, 4, sp<Uploader::Array<element_index_t>>::make(sp<IndexArray::Vector>::make(std::vector<element_index_t>({0, 2, 1, 2, 3, 1})))); };
}

SharedBuffer::Concat::Concat(size_t primitiveCount, size_t vertexCount, sp<Uploader> indices)
    : Uploader(primitiveCount * indices->size()), _primitive_count(primitiveCount), _vertex_count(vertexCount), _indices(std::move(indices))
{
}

void SharedBuffer::Concat::upload(Writable& uploader)
{
    size_t size = _indices->size();
    size_t length = size / sizeof(element_index_t);
    size_t offset = 0;
    std::vector<element_index_t> indices(length);
    element_index_t* buf = &indices.front();

    WritableMemory writer(buf);
    _indices->upload(writer);

    for(size_t i = 0; i < _primitive_count; ++i, offset += size)
    {
        if(i != 0)
            for(size_t j = 0; j < length; ++j)
                buf[j] += static_cast<element_index_t>(_vertex_count);
        uploader.write(buf, static_cast<uint32_t>(size), static_cast<uint32_t>(offset));
    }
}

SharedBuffer::Degenerate::Degenerate(size_t primitiveCount, size_t vertexCount, sp<Uploader> indices)
    : Uploader(primitiveCount * indices->size() + 2 *  sizeof(element_index_t) * (primitiveCount - 1)), _primitive_count(primitiveCount), _vertex_count(vertexCount), _indices(std::move(indices))
{
}

void SharedBuffer::Degenerate::upload(Writable& uploader)
{
    size_t size = _indices->size();
    size_t length = size / sizeof(element_index_t);
    uint32_t offset = 0;
    std::vector<element_index_t> indices(length + 2);
    element_index_t* buf = &indices.front();

    WritableMemory writer(buf);
    _indices->upload(writer);

    for(size_t i = 0; i < _primitive_count; ++i, offset += (size + 2 * sizeof(element_index_t)))
    {
        if(i == _primitive_count - 1)
            uploader.write(buf, static_cast<uint32_t>(size), offset);
        else
        {
            buf[length] = buf[length - 1];
            buf[length + 1] = static_cast<element_index_t>(buf[0] + _vertex_count);
            uploader.write(buf, static_cast<uint32_t>(size + 2 * sizeof(element_index_t)), offset);
            for(size_t j = 0; j < length; ++j)
                buf[j] += static_cast<element_index_t>(_vertex_count);
        }
    }
}

}
