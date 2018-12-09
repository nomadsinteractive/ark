#include "renderer/opengl/util/gl_index_buffers.h"

#include "core/impl/array/dynamic_array.h"

#include "renderer/base/gl_resource_manager.h"

namespace ark {

GLIndexBuffers::NinePatch::NinePatch(size_t objectCount)
    : Uploader(((28 + 2) * objectCount - 2) * sizeof(glindex_t)), _object_count(objectCount),
      _boiler_plate({0, 4, 1, 5, 2, 6, 3, 7, 7, 4, 4, 8, 5, 9, 6, 10, 7, 11, 11, 8, 8, 12, 9, 13, 10, 14, 11, 15}) {
    DASSERT(_object_count);
}

void GLIndexBuffers::NinePatch::upload(const Buffer::UploadFunc& uploader)
{
    const size_t bolierPlateLength = _boiler_plate.length();
    bytearray array = sp<DynamicArray<uint8_t>>::make(size());
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

Buffer::UploadMakerFunc GLIndexBuffers::NinePatch::maker()
{
    return [](size_t objectCount)->sp<Buffer::Uploader> { return sp<NinePatch>::make(objectCount); };
}

GLIndexBuffers::Quads::Quads(size_t objectCount)
    : Uploader(objectCount * 6 * sizeof(glindex_t)), _object_count(objectCount)
{
}

void GLIndexBuffers::Quads::upload(const Buffer::UploadFunc& uploader)
{
    bytearray result = sp<DynamicArray<uint8_t>>::make(size());

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

Buffer::UploadMakerFunc GLIndexBuffers::Quads::maker()
{
    return [](size_t objectCount)->sp<Buffer::Uploader> { return sp<Quads>::make(objectCount); };
}

GLIndexBuffers::Points::Points(size_t objectCount)
    : Uploader(objectCount * sizeof(glindex_t)), _object_count(objectCount)
{
}

void GLIndexBuffers::Points::upload(const Buffer::UploadFunc& uploader)
{
    const auto result = sp<DynamicArray<glindex_t>>::make(_object_count);

    glindex_t* buf = result->buf();
    size_t idx = 0;
    for(size_t i = 0; i < _object_count; i ++) {
        glindex_t offset = static_cast<glindex_t>(i);
        buf[idx++] = offset;
    }
    uploader(result->buf(), result->size());
}

Buffer::UploadMakerFunc GLIndexBuffers::Points::maker()
{
    return [](size_t objectCount)->sp<Buffer::Uploader> { return sp<Points>::make(objectCount); };
}

Buffer::Snapshot GLIndexBuffers::makeGLBufferSnapshot(GLResourceManager& resourceManager, Buffer::Name name, size_t objectCount)
{
    const size_t warningLimit = 10000;
    DWARN(objectCount < warningLimit, "Object count(%d) exceeding warning limit(%d). You can make the limit larger if you're sure what you're doing", objectCount, warningLimit);
    switch(name)
    {
    case Buffer::NAME_NINE_PATCH:
        return resourceManager.makeGLBufferSnapshot(name, NinePatch::maker(), objectCount * 2, (objectCount * 30 - 2) * sizeof(glindex_t));
    case Buffer::NAME_POINTS:
        return resourceManager.makeGLBufferSnapshot(name, Points::maker(), objectCount * 2, objectCount * sizeof(glindex_t));
    case Buffer::NAME_QUADS:
        return resourceManager.makeGLBufferSnapshot(name, Quads::maker(), objectCount * 2, objectCount * 6 * sizeof(glindex_t));
    default:
        DFATAL("Unknown GLBufferName %d", name);
    }
    return Buffer::Snapshot();
}

}
