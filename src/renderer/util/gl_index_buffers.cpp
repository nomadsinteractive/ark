#include "renderer/util/gl_index_buffers.h"

#include "core/impl/array/dynamic_array.h"

#include "renderer/base/gl_resource_manager.h"

namespace ark {

GLIndexBuffers::NinePatch::NinePatch(size_t objectCount)
    : _object_count(objectCount), _boiler_plate({0, 4, 1, 5, 2, 6, 3, 7, 7, 4, 4, 8, 5, 9, 6, 10, 7, 11, 11, 8, 8, 12, 9, 13, 10, 14, 11, 15}) {
    NOT_NULL(_object_count);
}

size_t GLIndexBuffers::NinePatch::size()
{
    return ((_boiler_plate.length() + 2) * _object_count - 2) * sizeof(glindex_t);
}

void GLIndexBuffers::NinePatch::upload(const GLBuffer::UploadFunc& uploader)
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

GLBuffer::UploadMakerFunc GLIndexBuffers::NinePatch::maker()
{
    return [](size_t size)->sp<GLBuffer::Uploader> { return sp<NinePatch>::make((size / sizeof(glindex_t) + 2) / 30); };
}

GLIndexBuffers::Quads::Quads(size_t objectCount)
    : _object_count(objectCount)
{
}

size_t GLIndexBuffers::Quads::size()
{
    return _object_count * 6 * sizeof(glindex_t);
}

void GLIndexBuffers::Quads::upload(const GLBuffer::UploadFunc& uploader)
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

GLBuffer::UploadMakerFunc GLIndexBuffers::Quads::maker()
{
    return [](size_t size)->sp<GLBuffer::Uploader> { return sp<Quads>::make(size / sizeof(glindex_t) / 6); };
}

GLIndexBuffers::Points::Points(size_t objectCount)
    : _object_count(objectCount)
{
}

size_t GLIndexBuffers::Points::size()
{
    return _object_count * sizeof(glindex_t);
}

void GLIndexBuffers::Points::upload(const GLBuffer::UploadFunc& uploader)
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

GLBuffer::UploadMakerFunc GLIndexBuffers::Points::maker()
{
    return [](size_t size)->sp<GLBuffer::Uploader> { return sp<Points>::make(size / sizeof(glindex_t)); };
}

GLBuffer::Snapshot GLIndexBuffers::makeGLBufferSnapshot(GLResourceManager& resourceManager, GLBuffer::Name name, size_t objectCount)
{
    switch(name)
    {
    case GLBuffer::NAME_NINE_PATCH:
        return resourceManager.makeGLBufferSnapshot(name, NinePatch::maker(), (objectCount * 30 - 2) * sizeof(glindex_t));
    case GLBuffer::NAME_POINTS:
        return resourceManager.makeGLBufferSnapshot(name, Points::maker(), objectCount * sizeof(glindex_t));
    case GLBuffer::NAME_QUADS:
        return resourceManager.makeGLBufferSnapshot(name, Quads::maker(), objectCount * 6 * sizeof(glindex_t));
    default:
        DFATAL("Unknown GLBufferName %d", name);
    }
    return GLBuffer::Snapshot();
}

}
