#include "renderer/impl/renderer/skybox.h"

#include "core/base/api.h"

#include "graphics/base/render_request.h"
#include "graphics/base/size.h"

#include "renderer/base/buffer.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/snippet_delegate.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/util/render_util.h"

namespace ark {

Skybox::Skybox(const sp<Size>& size, const sp<Shader>& shader, const sp<Texture>& texture, RenderController& renderController)
    : _size(size), _shader(shader), _shader_bindings(shader->makeBindings(ModelLoader::RENDER_MODE_TRIANGLES, PipelineBindings::RENDER_PROCEDURE_DRAW_ELEMENTS)),
      _vertices(renderController.makeVertexBuffer(Buffer::USAGE_STATIC, sp<ByteArrayUploader>::make(makeUnitCubeVertices(renderController)))),
      _index_buffer(renderController.getNamedBuffer(SharedBuffer::NAME_QUADS)->snapshot(renderController, 6))
{
    _shader_bindings->pipelineBindings()->bindSampler(texture);
}

void Skybox::render(RenderRequest& renderRequest, const V3& /*position*/)
{
    DrawingContext drawingContext(_shader_bindings, _shader_bindings->attachments(), _shader->takeUBOSnapshot(renderRequest), _vertices.snapshot(), _index_buffer, DrawingContext::ParamDrawElements(0, _index_buffer.length<element_index_t>()));
    renderRequest.addRequest(drawingContext.toRenderCommand(renderRequest));
}

const sp<Size>& Skybox::size()
{
    return _size;
}

sp<ByteArray> Skybox::makeUnitCubeVertices(RenderController& renderController) const
{
    return RenderUtil::makeUnitCubeVertices(renderController.renderEngine()->context()->coordinateSystem() == Ark::COORDINATE_SYSTEM_LHS);
}

Skybox::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _size(factory.ensureBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _shader(Shader::fromDocument(factory, manifest, resourceLoaderContext, "shaders/skybox.vert", "shaders/skybox.frag")),
      _texture(factory.ensureBuilder<Texture>(manifest, Constants::Attributes::TEXTURE))
{
}

sp<Renderer> Skybox::BUILDER::build(const Scope& args)
{
    const sp<Size> size = _size->build(args);
    return sp<Skybox>::make(size, _shader->build(args), _texture->build(args), _resource_loader_context->renderController());
}

}
