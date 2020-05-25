#include "renderer/base/shared_buffer.h"

#include "core/impl/writable/writable_memory.h"

#include "renderer/base/render_controller.h"

namespace ark {

SharedBuffer::SharedBuffer::SharedBuffer(const Buffer& buffer, Uploader::MakerFunc maker, std::function<size_t(size_t)> sizeCalculator)
    : _buffer(buffer), _maker(std::move(maker)), _size_calculator(std::move(sizeCalculator)), _object_count(0)
{
}

const Buffer& SharedBuffer::SharedBuffer::buffer() const
{
    return _buffer;
}

Buffer::Snapshot SharedBuffer::SharedBuffer::snapshot(RenderController& renderController, size_t objectCount, size_t reservedIfInsufficient)
{
    const size_t warningLimit = 10000;
    DWARN(objectCount < warningLimit, "Object count(%d) exceeding warning limit(%d). You can make the limit larger if you're sure what you're doing", objectCount, warningLimit);
    size_t size = _size_calculator(objectCount);
    if(_object_count < objectCount)
    {
        _object_count = objectCount + reservedIfInsufficient;
        const sp<Uploader> uploader = _maker(_object_count);
        DCHECK(uploader && uploader->size() >= size, "Making Uploader failed, object-count: %d, uploader-size: %d, required-size: %d", _object_count, uploader ? uploader->size() : 0, size);
        renderController.upload(_buffer.delegate(), uploader, RenderController::US_RELOAD);
    }
    return _buffer.snapshot(size);
}

Uploader::MakerFunc SharedBuffer::Quads::maker()
{
    return [](size_t objectCount)->sp<Uploader> { return sp<Concat>::make(objectCount, 4, sp<Uploader::Array<element_index_t>>::make(sp<IndexArray::Vector>::make(std::vector<element_index_t>({0, 2, 1, 2, 3, 1})))); };
}

SharedBuffer::Concat::Concat(size_t objectCount, size_t vertexCount, const sp<Uploader>& indices)
    : Uploader(objectCount * indices->size()), _object_count(objectCount), _vertex_count(vertexCount), _indices(indices)
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

    for(size_t i = 0; i < _object_count; ++i, offset += size)
    {
        if(i != 0)
            for(size_t j = 0; j < length; ++j)
                buf[j] += static_cast<element_index_t>(_vertex_count);
        uploader.write(buf, static_cast<uint32_t>(size), static_cast<uint32_t>(offset));
    }
}

SharedBuffer::Degenerate::Degenerate(size_t objectCount, size_t vertexCount, const sp<Uploader>& indices)
    : Uploader(objectCount * indices->size() + 2 *  sizeof(element_index_t) * (objectCount - 1)), _object_count(objectCount), _vertex_count(vertexCount), _indices(indices)
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

    for(size_t i = 0; i < _object_count; ++i, offset += (size + 2 * sizeof(element_index_t)))
    {
        if(i == _object_count - 1)
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
