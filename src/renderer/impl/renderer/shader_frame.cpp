#include "renderer/impl/renderer/shader_frame.h"

#include "core/base/api.h"
#include "core/base/memory_pool.h"
#include "core/impl/array/fixed_array.h"

#include "graphics/base/render_request.h"
#include "graphics/base/size.h"
#include "graphics/impl/vec/vec2_impl.h"

#include "renderer/base/buffer.h"
#include "renderer/base/resource_manager.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/ubo.h"
#include "renderer/inf/uploader.h"
#include "renderer/util/index_buffers.h"

#include "renderer/opengl/render_command/gl_draw_elements.h"

namespace ark {

ShaderFrame::ShaderFrame(const sp<Size>& size, const sp<Shader>& shader, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _size(size), _render_controller(resourceLoaderContext->renderController()), _shader(shader),
      _object_pool(resourceLoaderContext->objectPool()), _memory_pool(resourceLoaderContext->memoryPool()),
      _shader_bindings(sp<ShaderBindings>::make(_render_controller, shader)),
      _array_buffer(_shader_bindings->arrayBuffer())
{
}

void ShaderFrame::render(RenderRequest& renderRequest, float x, float y)
{
    const Buffer::Snapshot indexBuffer = IndexBuffers::makeBufferSnapshot(_render_controller, Buffer::NAME_QUADS, 1);
    const sp<Uploader> uploader = _object_pool->obtain<ByteArrayUploader>(getArrayBuffer(x, y));
    renderRequest.addRequest(_object_pool->obtain<opengl::GLDrawElements>(DrawingContext(_shader_bindings, _shader->snapshot(_memory_pool), _array_buffer.snapshot(uploader), indexBuffer, 0), _shader, GL_TRIANGLES));
}

const SafePtr<Size>& ShaderFrame::size()
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
      _shader(Shader::fromDocument(factory, manifest, resourceLoaderContext))
{
}

sp<Renderer> ShaderFrame::BUILDER::build(const sp<Scope>& args)
{
    const sp<Size> size = _size->build(args);
    const sp<Shader> shader = _shader->build(args);
    return sp<ShaderFrame>::make(size, shader, _resource_loader_context);
}

}
