#include "renderer/base/gl_resource_manager.h"

#include "core/inf/variable.h"
#include "core/impl/array/fixed_array.h"
#include "core/impl/array/dynamic_array.h"
#include "core/types/weak_ptr.h"
#include "core/util/log.h"

#include "renderer/base/gl_buffer.h"
#include "renderer/base/gl_context.h"
#include "renderer/base/gl_recycler.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_snippet_delegate.h"
#include "renderer/base/gl_texture.h"
#include "renderer/base/graphics_context.h"
#include "renderer/impl/resource/gl_texture_resource.h"
#include "renderer/inf/gl_snippet.h"

#include "platform/gl/gl.h"

namespace ark {

namespace {

class NinePatchIndexArrayVariable : public Variable<bytearray> {
public:
    NinePatchIndexArrayVariable(uint32_t objectCount)
        : _object_count(objectCount), _boiler_plate({0, 4, 1, 5, 2, 6, 3, 7, 7, 4, 4, 8, 5, 9, 6, 10, 7, 11, 11, 8, 8, 12, 9, 13, 10, 14, 11, 15}) {
    }

    virtual bytearray val() override {
        const uint32_t bolierPlateLength = _boiler_plate.length();
        bytearray array = sp<DynamicArray<uint8_t>>::make((bolierPlateLength + 2) * _object_count * 2 - 4);
        uint16_t* buf = reinterpret_cast<uint16_t*>(array->array());
        uint16_t* src = _boiler_plate.array();
        for(uint32_t i = 0; i < _object_count; i ++) {
            for(uint32_t j = 0; j < bolierPlateLength; j ++)
                buf[j] = static_cast<uint16_t>(src[j] + i * 16);
            if(i + 1 != _object_count) {
                buf[bolierPlateLength] = static_cast<uint16_t>(15 + i * 16);
                buf[bolierPlateLength + 1] = static_cast<uint16_t>((i + 1) * 16);
            }
            buf += ((bolierPlateLength + 2));
        }
        return array;
    }

private:
    uint32_t _object_count;
    FixedArray<uint16_t, 28> _boiler_plate;
};

class TrianglesIndexArrayVariable : public Variable<bytearray> {
public:
    TrianglesIndexArrayVariable(uint32_t objectCount)
        : _object_count(objectCount) {
    }

    bytearray val() {
        bytearray result = sp<DynamicArray<uint8_t>>::make(_object_count * 12);

        uint16_t* buf = reinterpret_cast<uint16_t*>(result->array());
        uint32_t idx = 0;
        for(uint32_t i = 0; i < _object_count; i ++) {
            uint16_t offset = static_cast<uint16_t>(i * 4);

            buf[idx++] = offset;
            buf[idx++] = offset + 2;
            buf[idx++] = offset + 1;
            buf[idx++] = offset + 2;
            buf[idx++] = offset + 3;
            buf[idx++] = offset + 1;

        }
        return result;
    }

private:
    uint32_t _object_count;
};

class PointIndexArrayVariable : public Variable<bytearray> {
public:
    PointIndexArrayVariable(uint32_t objectCount)
        : _object_count(objectCount) {
    }

    bytearray val() {
        const bytearray result = sp<DynamicArray<uint8_t>>::make(_object_count * 2);

        uint16_t* buf = reinterpret_cast<uint16_t*>(result->array());
        uint32_t idx = 0;
        for(uint32_t i = 0; i < _object_count; i ++) {
            uint16_t offset = static_cast<uint16_t>(i);
            buf[idx++] = offset;

        }
        return result;
    }

private:
    uint32_t _object_count;
};

}

GLResourceManager::GLResourceManager(const sp<Dictionary<bitmap>>& bitmapLoader, const sp<Dictionary<bitmap>>& bitmapBoundsLoader)
    : _recycler(sp<GLRecycler>::make()), _gl_texture_loader(sp<GLTextureResource>::make(_recycler, bitmapLoader, bitmapBoundsLoader)), _tick(0)
{
}

GLResourceManager::~GLResourceManager()
{
}

void GLResourceManager::onSurfaceReady(GraphicsContext& graphicsContext)
{
    doRecycling(graphicsContext);
    doSurfaceReady(graphicsContext);
}

void GLResourceManager::onDrawFrame(GraphicsContext& graphicsContext)
{
    for(const PreparingGLResource& resource : _preparing_items.clear())
        if(!resource._resource.isExpired())
        {
            resource._resource.prepare(graphicsContext, resource._strategy == PS_ONCE_FORCE);
            if(resource._strategy == PS_ONCE_AND_ON_SURFACE_READY)
                _on_surface_ready_items.insert(resource._resource);
        }

    uint32_t m = (++_tick) % 301;
    if(m == 0)
        doRecycling(graphicsContext);
    else if (m == 150)
        _recycler->doRecycling(graphicsContext);
}

void GLResourceManager::prepare(const sp<GLResource>& resource, PreparingStrategy strategy)
{
    switch(strategy)
    {
    case PS_ONCE_AND_ON_SURFACE_READY:
    case PS_ONCE:
    case PS_ONCE_FORCE:
        _preparing_items.push(PreparingGLResource(resource, strategy));
        break;
    case PS_ON_SURFACE_READY:
        _on_surface_ready_items.insert(resource);
        break;
    }
}

void GLResourceManager::prepare(const GLBuffer& buffer, GLResourceManager::PreparingStrategy strategy)
{
    prepare(buffer._stub, strategy);
}

void GLResourceManager::recycle(const sp<GLResource>& resource) const
{
    _recycler->recycle(resource);
}

GLBuffer GLResourceManager::getGLIndexBuffer(GLResourceManager::BufferName bufferName, uint32_t bufferLength)
{
    if(!bufferLength)
        return GLBuffer();

    if(!_static_buffers[bufferName] || _static_buffers[bufferName].length<uint16_t>() < bufferLength)
    {
        _static_buffers[bufferName] = createStaticBuffer(bufferName, bufferLength);
        prepare(_static_buffers[bufferName], PS_ONCE_AND_ON_SURFACE_READY);
    }

    GLBuffer indexBuffer = _static_buffers[bufferName];
    indexBuffer.setSize(bufferLength * sizeof(uint16_t));
    return indexBuffer;
}

sp<GLTexture> GLResourceManager::loadGLTexture(const String& name)
{
    const sp<GLTexture> texture = _gl_texture_loader->get(name);
    DCHECK(texture, "Texture \"%s\" not loaded", name.c_str());
    prepare(texture, PS_ONCE_AND_ON_SURFACE_READY);
    return texture;
}

sp<GLTexture> GLResourceManager::createGLTexture(uint32_t width, uint32_t height, const sp<Variable<bitmap>>& bitmapVariable)
{
    sp<GLTexture> texture = sp<GLTexture>::make(_recycler, width, height, bitmapVariable);
    prepare(texture, PS_ONCE_AND_ON_SURFACE_READY);
    return texture;
}

GLBuffer GLResourceManager::createGLBuffer(const sp<GLBuffer::Uploader>& uploader, GLenum type, GLenum usage)
{
    return GLBuffer(_recycler, uploader, type, usage);
}

const sp<GLRecycler>& GLResourceManager::recycler() const
{
    return _recycler;
}

GLBuffer GLResourceManager::createStaticBuffer(GLResourceManager::BufferName bufferName, uint32_t bufferLength) const
{
    const uint32_t bufferSize = bufferLength * sizeof(uint16_t);
    switch(bufferName)
    {
        case BUFFER_NAME_TRANGLES:
            DCHECK(bufferLength % 6 == 0, "Length of index array for triangles should be 6 times of an integer.");
            return GLBuffer(_recycler, sp<Variable<bytearray>>::adopt(new TrianglesIndexArrayVariable(bufferLength / 3)), GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, bufferSize);
        case BUFFER_NAME_NINE_PATCH:
            DCHECK((bufferLength + 2) % 30 == 0, "Illegal length of nine patch index array.");
            return GLBuffer(_recycler, sp<Variable<bytearray>>::adopt(new NinePatchIndexArrayVariable((bufferLength + 2) / 30)), GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, bufferSize);
        case BUFFER_NAME_POINTS:
            return GLBuffer(_recycler, sp<Variable<bytearray>>::adopt(new PointIndexArrayVariable(static_cast<uint32_t>(bufferLength * 1.4f))), GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, bufferSize);
        default:
            break;
    }
    DFATAL("Can not create static buffer");
    return GLBuffer();
}

void GLResourceManager::doRecycling(GraphicsContext& graphicsContext)
{
    for(auto iter = _on_surface_ready_items.begin(); iter != _on_surface_ready_items.end();)
    {
        const ExpirableGLResource& resource = *iter;
        if(resource.isExpired())
        {
            resource.recycle(graphicsContext);
            iter = _on_surface_ready_items.erase(iter);
        }
        else
            ++iter;
    }
}

void GLResourceManager::doSurfaceReady(GraphicsContext& graphicsContext)
{
    for(const ExpirableGLResource& resource :_on_surface_ready_items)
        resource.recycle(graphicsContext);

    for(const ExpirableGLResource& resource :_on_surface_ready_items)
        resource.prepare(graphicsContext, true);
}

GLResourceManager::ExpirableGLResource::ExpirableGLResource(const sp<GLResource>& resource)
    : _resource(resource) {
}

GLResourceManager::ExpirableGLResource::ExpirableGLResource(const GLResourceManager::ExpirableGLResource& other)
    : _resource(other._resource) {
}

const sp<GLResource>& GLResourceManager::ExpirableGLResource::resource() const
{
    return _resource;
}

bool GLResourceManager::ExpirableGLResource::isExpired() const
{
    return _resource.unique();
}

void GLResourceManager::ExpirableGLResource::prepare(GraphicsContext& graphicsContext, bool force) const
{
    if(force || !_resource->id())
        _resource->prepare(graphicsContext);
}

void GLResourceManager::ExpirableGLResource::recycle(GraphicsContext& graphicsContext) const
{
    _resource->recycle(graphicsContext);
}

bool GLResourceManager::ExpirableGLResource::operator <(const GLResourceManager::ExpirableGLResource& other) const
{
    return _resource < other._resource;
}

GLResourceManager::PreparingGLResource::PreparingGLResource(const GLResourceManager::ExpirableGLResource& resource, GLResourceManager::PreparingStrategy strategy)
    : _resource(resource), _strategy(strategy)
{
}

GLResourceManager::PreparingGLResource::PreparingGLResource(const GLResourceManager::PreparingGLResource& other)
    : _resource(other._resource), _strategy(other._strategy)
{
}

bool GLResourceManager::PreparingGLResource::operator <(const GLResourceManager::PreparingGLResource& other) const
{
    return _resource < other._resource;
}

}
