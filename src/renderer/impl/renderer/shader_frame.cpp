#include "renderer/impl/renderer/shader_frame.h"

#include "core/base/api.h"
#include "core/base/memory_pool.h"
#include "core/impl/array/fixed_array.h"

#include "graphics/base/render_request.h"
#include "graphics/base/size.h"
#include "graphics/impl/vec/vec2_impl.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_shader_bindings.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/render_command/draw_elements.h"
#include "renderer/impl/gl_snippet/gl_snippet_textures.h"

namespace ark {

ShaderFrame::ShaderFrame(const sp<Size>& size, const sp<GLShader>& shader, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _size(size), _resource_manager(resourceLoaderContext->glResourceManager()), _shader(shader),
      _array_buffer(_resource_manager->createDynamicArrayBuffer()),
      _object_pool(resourceLoaderContext->objectPool()), _memory_pool(resourceLoaderContext->memoryPool()),
      _shader_bindings(sp<GLShaderBindings>::make(shader, _array_buffer))
{
}

void ShaderFrame::render(RenderRequest& renderRequest, float x, float y)
{
    const GLBuffer indexBuffer = _resource_manager->getGLIndexBuffer(GLResourceManager::BUFFER_NAME_TRANGLES, 6);
    if(indexBuffer)
    {
        const sp<GLBuffer::Uploader> uploader = _object_pool->obtain<GLBuffer::ByteArrayUploader>(getArrayBuffer(x, y));
        int32_t count = indexBuffer.length<glindex_t>();
        renderRequest.addRequest(_object_pool->obtain<DrawElements>(GLDrawingContext(_shader_bindings, _array_buffer.snapshot(uploader), indexBuffer.snapshot(), GL_TRIANGLES), _shader, count));
    }
}

const sp<Size>& ShaderFrame::size()
{
    return _size;
}

bytearray ShaderFrame::getArrayBuffer(float x, float y) const
{
    float top = y + _size->height(), bottom = y;
    uint16_t uvtop = 0xffff, uvbottom = 0;
    FixedArray<float, 16> buffer({x, bottom, 0, 0, x, top, 0, 0, x + _size->width(), bottom, 0, 0, x + _size->width(), top, 0, 0});
    uint16_t* ip = reinterpret_cast<uint16_t*>(buffer.buf());
    ip[6] = 0;
    ip[7] = uvbottom;
    ip[14] = 0;
    ip[15] = uvtop;
    ip[22] = 0xffff;
    ip[23] = uvbottom;
    ip[30] = 0xffff;
    ip[31] = uvtop;
    const bytearray preallocated = _memory_pool->allocate(64);
    memcpy(preallocated->buf(), buffer.buf(), 16 * sizeof(GLfloat));
    return preallocated;
}

ShaderFrame::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _size(factory.ensureBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _shader(GLShader::fromDocument(factory, manifest, resourceLoaderContext))
{
}

sp<Renderer> ShaderFrame::BUILDER::build(const sp<Scope>& args)
{
    const sp<Size> size = _size->build(args);
    const sp<GLShader> shader = _shader->build(args);
    return sp<ShaderFrame>::make(size, shader, _resource_loader_context);
}

}
