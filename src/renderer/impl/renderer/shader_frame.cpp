#include "renderer/impl/renderer/shader_frame.h"

#include "core/base/api.h"
#include "core/base/memory_pool.h"
#include "core/impl/array/fixed_array.h"
#include "core/util/documents.h"

#include "graphics/base/render_command_pipeline.h"
#include "graphics/base/render_request.h"
#include "graphics/base/size.h"
#include "graphics/base/vec2.h"

#include "renderer/base/gl_buffer.h"
#include "renderer/base/gl_program.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/render_command/draw_elements.h"
#include "renderer/impl/gl_snippet/gl_snippet_textures.h"
#include "renderer/inf/gl_model.h"

namespace ark {

namespace {

class GLModelShader : public GLModel {
public:
    GLModelShader(const sp<Size>& size)
        : _size(size) {
    }

    virtual bytearray getArrayBuffer(MemoryPool& memoryPool, const LayerContext::Snapshot& /*renderContext*/, float x, float y) override {
        float top = g_isOriginBottom ? y + _size->height() : y, bottom = g_isOriginBottom ? y : y + _size->height();
        uint16_t uvtop = g_isOriginBottom ? 0xffff : 0, uvbottom = g_isOriginBottom ? 0 : 0xffff;
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
        const bytearray preallocated = memoryPool.allocate(64);
        memcpy(preallocated->buf(), buffer.buf(), 16 * sizeof(GLfloat));
        return preallocated;
    }

    virtual GLBuffer getIndexBuffer(GLResourceManager& glResourceManager, const LayerContext::Snapshot& /*renderContext*/) override {
        return glResourceManager.getGLIndexBuffer(GLResourceManager::BUFFER_NAME_TRANGLES, 6);
    }

    virtual uint32_t mode() const override {
        return static_cast<uint32_t>(GL_TRIANGLES);
    }

private:
    sp<Size> _size;

};

}

ShaderFrame::ShaderFrame(const sp<Size>& size, const sp<GLShader>& shader, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _size(size), _elements(shader, nullptr, sp<GLModelShader>::make(size), resourceLoaderContext), _render_context(resourceLoaderContext->memoryPool())
{
}

void ShaderFrame::render(RenderRequest& renderRequest, float x, float y)
{
    const sp<RenderCommand> renderCommand = _elements.render(_render_context.snapshot(), x, y);
    if(renderCommand)
        renderRequest.addRequest(renderCommand);
}

const sp<Size>& ShaderFrame::size()
{
    return _size;
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
