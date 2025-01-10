#include "renderer/impl/renderer/shader_frame.h"

#include "core/types/global.h"

#include "graphics/base/render_request.h"
#include "graphics/components/size.h"

#include "renderer/base/buffer.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/shader.h"
#include "renderer/base/pipeline_bindings.h"

namespace ark {

ShaderFrame::ShaderFrame(sp<Vec3> size, const sp<Shader>& shader, RenderController& renderController)
    : _size(std::move(size)), _shader(shader), _pipeline_bindings(shader->makeBindings(Buffer(), Enum::RENDER_MODE_TRIANGLES, Enum::DRAW_PROCEDURE_DRAW_ELEMENTS)),
      _vertex_buffer(renderController.makeVertexBuffer()), _ib_snapshot(renderController.getSharedPrimitiveIndexBuffer(Global<Constants>()->MODEL_UNIT_QUAD_RHS, false)->snapshot(renderController, 1))
{
}

void ShaderFrame::render(RenderRequest& renderRequest, const V3& position)
{
    DrawingContext drawingContext(_pipeline_bindings, _shader->takeBufferSnapshot(renderRequest, false), _pipeline_bindings->attachments(),
                                  _vertex_buffer.snapshot(getVertexBuffer(renderRequest, position)), _ib_snapshot, 6, DrawingParams::DrawElements{0});
    renderRequest.addRenderCommand(drawingContext.toRenderCommand(renderRequest));
}

const SafeVar<Vec3>& ShaderFrame::size()
{
    return _size;
}

ByteArray::Borrowed ShaderFrame::getVertexBuffer(RenderRequest& renderRequest, const V3& position) const
{
    const V3 size = _size.val();
    float x = position.x(), y = position.y(), z = position.z();
    float top = y + size.y(), bottom = y;
    uint16_t uvtop = 0xffff, uvbottom = 0;
    Array<float>::Fixed<16> buffer({x, bottom, z, 0, x, top, z, 0, x + size.x(), bottom, z, 0, x + size.x(), top, z, 0});
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
    : _resource_loader_context(resourceLoaderContext), _size(factory.ensureConcreteClassBuilder<Size>(manifest, constants::SIZE)),
      _shader(Shader::fromDocument(factory, manifest, resourceLoaderContext))
{
}

sp<Renderer> ShaderFrame::BUILDER::build(const Scope& args)
{
    const sp<Size> size = _size->build(args);
    const sp<Shader> shader = _shader->build(args);
    return sp<Renderer>::make<ShaderFrame>(size, shader, _resource_loader_context->renderController());
}

}
