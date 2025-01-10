#include "renderer/impl/renderer/skybox.h"

#include "core/util/uploader_type.h"
#include "core/types/global.h"

#include "graphics/base/render_request.h"
#include "graphics/components/size.h"

#include "renderer/base/buffer.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/shader.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/util/render_util.h"

namespace ark {

namespace {

sp<ByteArray> makeUnitCubeVertices(RenderController& renderController)
{
    return RenderUtil::makeUnitCubeVertices(renderController.renderEngine()->coordinateSystem() == Ark::COORDINATE_SYSTEM_LHS);
}

}

Skybox::Skybox(const sp<Shader>& shader, const sp<Texture>& texture, RenderController& renderController)
    : _shader(shader), _pipeline_bindings(shader->makeBindings(renderController.makeVertexBuffer({}, UploaderType::create(makeUnitCubeVertices(renderController))), Enum::RENDER_MODE_TRIANGLES, Enum::DRAW_PROCEDURE_DRAW_ELEMENTS)),
      _ib_snapshot(renderController.getSharedPrimitiveIndexBuffer(Global<Constants>()->MODEL_UNIT_QUAD_RHS, false)->snapshot(renderController, 6))
{
    _pipeline_bindings->pipelineDescriptor()->bindSampler(texture);
}

void Skybox::render(RenderRequest& renderRequest, const V3& /*position*/)
{
    DrawingContext drawingContext(_pipeline_bindings, _shader->takeBufferSnapshot(renderRequest, false), _pipeline_bindings->attachments(), _pipeline_bindings->vertices().snapshot(),
                                  _ib_snapshot, 36, DrawingParams::DrawElements{0});
    renderRequest.addRenderCommand(drawingContext.toRenderCommand(renderRequest));
}

Skybox::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _shader(Shader::fromDocument(factory, manifest, resourceLoaderContext, "shaders/skybox.vert", "shaders/skybox.frag")),
      _texture(factory.ensureBuilder<Texture>(manifest, constants::TEXTURE))
{
}

sp<Renderer> Skybox::BUILDER::build(const Scope& args)
{
    return sp<Renderer>::make<Skybox>(_shader->build(args), _texture->build(args), _resource_loader_context->renderController());
}

}
