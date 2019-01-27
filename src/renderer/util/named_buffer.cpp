#include "renderer/util/named_buffer.h"

#include "renderer/base/render_controller.h"
#include "renderer/base/resource_manager.h"

namespace ark {

NamedBuffer::NamedBuffer::NamedBuffer(const Buffer& buffer, Uploader::MakerFunc maker, std::function<size_t(size_t)> sizeCalculator)
    : _buffer(buffer), _maker(std::move(maker)), _size_calculator(std::move(sizeCalculator)), _object_count(0)
{
}

const Buffer& NamedBuffer::NamedBuffer::buffer() const
{
    return _buffer;
}

Buffer::Snapshot NamedBuffer::NamedBuffer::snapshot(ResourceManager& resourceManager, size_t objectCount)
{
    const size_t warningLimit = 10000;
    DWARN(objectCount < warningLimit, "Object count(%d) exceeding warning limit(%d). You can make the limit larger if you're sure what you're doing", objectCount, warningLimit);
    size_t size = _size_calculator(objectCount);
    if(_object_count < objectCount)
    {
        _object_count = objectCount;
        const sp<Uploader> uploader = _maker(objectCount);
        DCHECK(uploader && uploader->size() >= size, "Making Uploader failed, object-count: %d, uploader-size: %d, required-size: %d", objectCount, uploader ? uploader->size() : 0, size);

        resourceManager.upload(_buffer.delegate(), uploader, ResourceManager::US_RELOAD);
    }
    return _buffer.snapshot(size);
}

NamedBuffer::NinePatch::NinePatch(size_t objectCount)
    : Uploader(((28 + 2) * objectCount - 2) * sizeof(glindex_t)), _object_count(objectCount),
      _boiler_plate({0, 4, 1, 5, 2, 6, 3, 7, 7, 4, 4, 8, 5, 9, 6, 10, 7, 11, 11, 8, 8, 12, 9, 13, 10, 14, 11, 15}) {
    DASSERT(_object_count);
}

void NamedBuffer::NinePatch::upload(const Uploader::UploadFunc& uploader)
{
    const size_t bolierPlateLength = _boiler_plate.length();
    bytearray array = sp<ByteArray::Allocated>::make(size());
    glindex_t* buf = reinterpret_cast<glindex_t*>(array->buf());
    glindex_t* src = _boiler_plate.buf();
    for(size_t i = 0; i < _object_count; i ++) {
        for(size_t j = 0; j < bolierPlateLength; j ++)
            buf[j] = static_cast<glindex_t>(src[j] + i * 16);
        if(i + 1 != _object_count) {
            buf[bolierPlateLength] = static_cast<glindex_t>(15 + i * 16);
            buf[bolierPlateLength + 1] = static_cast<glindex_t>((i + 1) * 16);
        }
        buf += ((bolierPlateLength + 2));
    }

    uploader(array->buf(), array->length());
}

sp<NamedBuffer> NamedBuffer::NinePatch::make(const RenderController& renderController)
{
    return sp<NamedBuffer>::make(renderController.makeIndexBuffer(Buffer::USAGE_STATIC),
                                                   [](size_t objectCount)->sp<Uploader> { return sp<NinePatch>::make(objectCount); },
                                                   [](size_t objectCount)->size_t { return ((28 + 2) * objectCount - 2) * sizeof(glindex_t); });
}

NamedBuffer::Quads::Quads(size_t objectCount)
    : Uploader(objectCount * 6 * sizeof(glindex_t)), _object_count(objectCount)
{
}

void NamedBuffer::Quads::upload(const Uploader::UploadFunc& uploader)
{
    bytearray result = sp<ByteArray::Allocated>::make(size());

    glindex_t* buf = reinterpret_cast<glindex_t*>(result->buf());
    size_t idx = 0;
    for(size_t i = 0; i < _object_count; i ++) {
        glindex_t offset = static_cast<glindex_t>(i * 4);

        buf[idx++] = offset;
        buf[idx++] = offset + 2;
        buf[idx++] = offset + 1;
        buf[idx++] = offset + 2;
        buf[idx++] = offset + 3;
        buf[idx++] = offset + 1;
    }
    uploader(result->buf(), result->length());
}

Uploader::MakerFunc NamedBuffer::Quads::maker()
{
    return [](size_t objectCount)->sp<Uploader> { return sp<Quads>::make(objectCount); };
}

sp<NamedBuffer> NamedBuffer::Quads::make(const RenderController& renderController)
{
    return sp<NamedBuffer>::make(renderController.makeIndexBuffer(Buffer::USAGE_STATIC),
                                                   [](size_t objectCount)->sp<Uploader> { return sp<Quads>::make(objectCount); },
                                                   [](size_t objectCount)->size_t { return objectCount * 6 * sizeof(glindex_t); });
}

NamedBuffer::Points::Points(size_t objectCount)
    : Uploader(objectCount * sizeof(glindex_t)), _object_count(objectCount)
{
}

void NamedBuffer::Points::upload(const Uploader::UploadFunc& uploader)
{
    const auto result = sp<IndexArray::Allocated>::make(_object_count);

    glindex_t* buf = result->buf();
    size_t idx = 0;
    for(size_t i = 0; i < _object_count; i ++) {
        glindex_t offset = static_cast<glindex_t>(i);
        buf[idx++] = offset;
    }
    uploader(result->buf(), result->size());
}

sp<NamedBuffer> NamedBuffer::Points::make(const RenderController& renderController)
{
    return sp<NamedBuffer>::make(renderController.makeIndexBuffer(Buffer::USAGE_STATIC),
                                                   [](size_t objectCount)->sp<Uploader> { return sp<Points>::make(objectCount); },
                                                   [](size_t objectCount)->size_t { return objectCount * sizeof(glindex_t); });
}

}
