#include "renderer/impl/renderer/shader_frame.h"

#include "core/base/api.h"
#include "core/base/memory_pool.h"

#include "graphics/base/render_request.h"
#include "graphics/base/size.h"

#include "renderer/base/buffer.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/shared_indices.h"

namespace ark {

ShaderFrame::ShaderFrame(const sp<Size>& size, const sp<Shader>& shader, RenderController& renderController)
    : _size(size), _shader(shader), _shader_bindings(shader->makeBindings(Buffer(), ModelLoader::RENDER_MODE_TRIANGLES, PipelineBindings::RENDER_PROCEDURE_DRAW_ELEMENTS)),
      _vertex_buffer(renderController.makeVertexBuffer()), _ib_snapshot(renderController.getSharedIndices(RenderController::SHARED_INDICES_QUAD)->snapshot(renderController, 1))
{
}

void ShaderFrame::render(RenderRequest& renderRequest, const V3& position)
{
    DrawingContext drawingContext(_shader_bindings, _shader_bindings->attachments(), _shader->takeUBOSnapshot(renderRequest), _shader->takeSSBOSnapshot(renderRequest),
                                  _vertex_buffer.snapshot(getVertexBuffer(renderRequest, position)), _ib_snapshot, DrawingContextParams::DrawElements(0, 6));
    renderRequest.addRequest(drawingContext.toRenderCommand(renderRequest));
}

const sp<Size>& ShaderFrame::size()
{
    return _size;
}

ByteArray::Borrowed ShaderFrame::getVertexBuffer(RenderRequest& renderRequest, const V3& position) const
{
    float x = position.x(), y = position.y();
    float top = y + _size->heightAsFloat(), bottom = y;
    uint16_t uvtop = 0xffff, uvbottom = 0;
    Array<float>::Fixed<16> buffer({x, bottom, 0, 0, x, top, 0, 0, x + _size->widthAsFloat(), bottom, 0, 0, x + _size->widthAsFloat(), top, 0, 0});
    uint16_t* ip = reinterpret_cast<uint16_t*>(buffer.buf());
    ip[6] = 0;
    ip[7] = uvbottom;
    ip[14] = 0;
    ip[15] = uvtop;
    ip[22] = 0xffff;
    ip[23] = uvbottom;
    ip[30] = 0xffff;
    ip[31] = uvtop;
    ByteArray::Borrowed allocated = renderRequest.allocator().sbrkSpan(buffer.size());
    memcpy(allocated.buf(), buffer.buf(), buffer.size());
    return allocated;
}

ShaderFrame::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _size(factory.ensureConcreteClassBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _shader(Shader::fromDocument(factory, manifest, resourceLoaderContext))
{
}

sp<Renderer> ShaderFrame::BUILDER::build(const Scope& args)
{
    const sp<Size> size = _size->build(args);
    const sp<Shader> shader = _shader->build(args);
    return sp<ShaderFrame>::make(size, shader, _resource_loader_context->renderController());
}

}
