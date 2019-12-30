#include "renderer/util/named_buffer.h"

#include "renderer/base/render_controller.h"
#include "renderer/base/render_controller.h"

#include "renderer/base/model.h"

namespace ark {

NamedBuffer::NamedBuffer::NamedBuffer(const Buffer& buffer, Uploader::MakerFunc maker, std::function<size_t(size_t)> sizeCalculator)
    : _buffer(buffer), _maker(std::move(maker)), _size_calculator(std::move(sizeCalculator)), _object_count(0)
{
}

const Buffer& NamedBuffer::NamedBuffer::buffer() const
{
    return _buffer;
}

void NamedBuffer::reset()
{
    _object_count = 0;
}

Buffer::Snapshot NamedBuffer::NamedBuffer::snapshot(RenderController& renderController, size_t objectCount, size_t reservedIfInsufficient)
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

Uploader::MakerFunc NamedBuffer::NinePatch::maker()
{
    return [](size_t objectCount)->sp<Uploader> { return sp<Degenerate>::make(objectCount, 16, sp<IndexArray::Fixed<28>>::make(std::initializer_list<element_index_t>({0, 4, 1, 5, 2, 6, 3, 7, 7, 4, 4, 8, 5, 9, 6, 10, 7, 11, 11, 8, 8, 12, 9, 13, 10, 14, 11, 15}))); };
}

sp<NamedBuffer> NamedBuffer::NinePatch::make(RenderController& renderController)
{
    return sp<NamedBuffer>::make(renderController.makeIndexBuffer(Buffer::USAGE_STATIC),
                                                   maker(),
                                                   [](size_t objectCount)->size_t { return ((28 + 2) * objectCount - 2) * sizeof(element_index_t); });
}

Uploader::MakerFunc NamedBuffer::Quads::maker()
{
    return [](size_t objectCount)->sp<Uploader> { return sp<Concat>::make(objectCount, 4, sp<IndexArray::Vector>::make(std::vector<element_index_t>({0, 2, 1, 2, 3, 1}))); };
}

sp<NamedBuffer> NamedBuffer::Quads::make(RenderController& renderController)
{
    return sp<NamedBuffer>::make(renderController.makeIndexBuffer(Buffer::USAGE_STATIC),
                                                   maker(),
                                                   [](size_t objectCount)->size_t { return objectCount * 6 * sizeof(element_index_t); });
}

NamedBuffer::Points::Points(size_t objectCount)
    : Uploader(objectCount * sizeof(element_index_t)), _object_count(objectCount)
{
}

void NamedBuffer::Points::upload(const Uploader::UploadFunc& uploader)
{
    const auto result = sp<IndexArray::Allocated>::make(_object_count);

    element_index_t* buf = result->buf();
    size_t idx = 0;
    for(size_t i = 0; i < _object_count; i ++) {
        element_index_t offset = static_cast<element_index_t>(i);
        buf[idx++] = offset;
    }
    uploader(result->buf(), result->size(), 0);
}

sp<NamedBuffer> NamedBuffer::Points::make(RenderController& renderController)
{
    return sp<NamedBuffer>::make(renderController.makeIndexBuffer(Buffer::USAGE_STATIC),
                                                   [](size_t objectCount)->sp<Uploader> { return sp<Points>::make(objectCount); },
    [](size_t objectCount)->size_t { return objectCount * sizeof(element_index_t); });
}

NamedBuffer::Concat::Concat(size_t objectCount, size_t vertexCount, const array<element_index_t>& indices)
    : Uploader(objectCount * indices->size()), _object_count(objectCount), _vertex_count(vertexCount), _indices(indices)
{
}

void NamedBuffer::Concat::upload(const Uploader::UploadFunc& uploader)
{
    size_t length = _indices->length();
    size_t size = _indices->size();
    size_t offset = 0;
    std::vector<element_index_t> indices(length);
    element_index_t* buf = &indices.front();
    memcpy(buf, _indices->buf(), size);
    for(size_t i = 0; i < _object_count; ++i, offset += size)
    {
        if(i != 0)
            for(size_t j = 0; j < length; ++j)
                buf[j] += static_cast<element_index_t>(_vertex_count);
        uploader(buf, size, offset);
    }
}

NamedBuffer::Degenerate::Degenerate(size_t objectCount, size_t vertexCount, const array<element_index_t>& indices)
    : Uploader(objectCount * indices->size() + 2 *  sizeof(element_index_t) * (objectCount - 1)), _object_count(objectCount), _vertex_count(vertexCount), _indices(indices)
{
}

void NamedBuffer::Degenerate::upload(const Uploader::UploadFunc& uploader)
{
    size_t length = _indices->length();
    size_t size = _indices->size();
    size_t offset = 0;
    std::vector<element_index_t> indices(length + 2);
    element_index_t* buf = &indices.front();
    memcpy(buf, _indices->buf(), size);
    for(size_t i = 0; i < _object_count; ++i, offset += (size + 2 * sizeof(element_index_t)))
    {
        if(i == _object_count - 1)
            uploader(buf, size, offset);
        else
        {
            buf[length] = buf[length - 1];
            buf[length + 1] = static_cast<element_index_t>(buf[0] + _vertex_count);
            uploader(buf, size + 2 * sizeof(element_index_t), offset);
            for(size_t j = 0; j < length; ++j)
                buf[j] += static_cast<element_index_t>(_vertex_count);
        }
    }
}

}
