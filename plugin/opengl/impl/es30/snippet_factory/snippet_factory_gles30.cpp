#include "opengl/impl/es30/snippet_factory/snippet_factory_gles30.h"

#include "core/collection/traits.h"

#include "renderer/inf/snippet.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/pipeline_bindings.h"

#include "opengl/impl/es30/gl_resource/gl_vertex_array.h"

#include "platform/gl/gl.h"

namespace ark::plugin::opengl {

namespace {

class DrawEventsGLES30 final : public Snippet::DrawDecorator {
public:
    void preDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override {
        if(!context._vertices)
            return;

        const sp<GLVertexArray>& vertexArray = context._attachments->get<GLVertexArray>();
        uint64_t vertexArrayId = vertexArray ? vertexArray->id() : 0;
        if(!vertexArrayId) {
            const sp<Pipeline>& renderPipeline = context._bindings->ensureRenderPipeline(graphicsContext);
            sp<GLVertexArray> va = sp<GLVertexArray>::make(renderPipeline.cast<GLPipeline>(), context._vertices.delegate(), context._bindings);
            va->upload(graphicsContext);
            graphicsContext.renderController()->upload(va, RenderController::US_ON_SURFACE_READY);
            vertexArrayId = va->id();
            context._attachments->put(std::move(va));
        }
        glBindVertexArray(static_cast<GLuint>(vertexArrayId));
        if(context._indices)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(context._indices.id()));
    }

    void postDraw(GraphicsContext& /*graphicsContext*/, const DrawingContext& /*context*/) override {
        glBindVertexArray(0);
    }

};

class SnippetGLES30 final : public Snippet {
public:
    void preInitialize(PipelineBuildingContext& /*context*/) override {
    }

    void preCompile(GraphicsContext& /*graphicsContext*/, PipelineBuildingContext& context, const PipelineLayout& /*pipelineLayout*/) override {
        if(ShaderPreprocessor* vertex = context.tryGetRenderStage(Enum::SHADER_STAGE_BIT_VERTEX))
            vertex->_predefined_macros.push_back("#define gl_InstanceIndex gl_InstanceID");

        if(ShaderPreprocessor* fragment = context.tryGetRenderStage(Enum::SHADER_STAGE_BIT_FRAGMENT))
            fragment->linkNextStage("FragColor");
    }

    sp<DrawDecorator> makeDrawDecorator() override {
        return sp<DrawDecorator>::make<DrawEventsGLES30>();
    }
};

}

sp<Snippet> SnippetFactoryGLES30::createCoreSnippet()
{
    return sp<Snippet>::make<SnippetGLES30>();
}

}
