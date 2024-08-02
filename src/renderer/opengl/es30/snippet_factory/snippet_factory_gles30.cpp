#include "renderer/opengl/es30/snippet_factory/snippet_factory_gles30.h"

#include "core/collection/traits.h"

#include "renderer/inf/snippet.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/pipeline_bindings.h"

#include "renderer/opengl/es30/gl_resource/gl_vertex_array.h"

#include "platform/gl/gl.h"

namespace ark::gles30 {

namespace {

class DrawEventsGLES30 final : public Snippet::DrawEvents {
public:
    void preDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override {
        const sp<GLVertexArray>& vertexArray = context._attachments->get<GLVertexArray>();
        uint64_t vertexArrayId = vertexArray ? vertexArray->id() : 0;
        if(!vertexArrayId) {
            sp<GLVertexArray> va = sp<GLVertexArray>::make(context._pipeline_snapshot._bindings->getPipeline(graphicsContext), context._vertices.delegate(), context._pipeline_snapshot._bindings);
            va->upload(graphicsContext);
            graphicsContext.renderController()->upload(va, RenderController::US_ON_SURFACE_READY);
            vertexArrayId = va->id();
            context._attachments->put(std::move(va));
        }
        glBindVertexArray(static_cast<GLuint>(vertexArrayId));
        if(context._indices)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(context._indices.id()));
    }

    void postDraw(GraphicsContext& /*graphicsContext*/) override {
        glBindVertexArray(0);
    }

};

class SnippetGLES30 final : public Snippet {
public:
    void preInitialize(PipelineBuildingContext& /*context*/) override {
    }

    void preCompile(GraphicsContext& /*graphicsContext*/, PipelineBuildingContext& context, const PipelineLayout& /*pipelineLayout*/) override {
        if(ShaderPreprocessor* vertex = context.tryGetStage(PipelineInput::SHADER_STAGE_VERTEX))
            vertex->_predefined_macros.push_back("#define gl_InstanceIndex gl_InstanceID");

        if(ShaderPreprocessor* fragment = context.tryGetStage(PipelineInput::SHADER_STAGE_FRAGMENT))
            fragment->linkNextStage("FragColor");
    }

    sp<DrawEvents> makeDrawEvents() override {
        return sp<DrawEventsGLES30>::make();
    }
};

}

sp<Snippet> SnippetFactoryGLES30::createCoreSnippet()
{
    return sp<SnippetGLES30>::make();
}

}
