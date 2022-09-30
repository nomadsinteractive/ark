#include "renderer/opengl/es30/snippet_factory/snippet_factory_gles30.h"

#include "core/collection/by_type.h"

#include "renderer/inf/snippet.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/shader_bindings.h"

#include "renderer/opengl/es30/gl_resource/gl_vertex_array.h"

#include "platform/gl/gl.h"

namespace ark {
namespace gles30 {

namespace {

class DrawEventsGLES30 : public Snippet::DrawEvents {
public:
    virtual void preDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override {
        const sp<GLVertexArray>& vertexArray = context._attachments->get<GLVertexArray>();
        uint64_t vertexArrayId = vertexArray ? vertexArray->id() : 0;
        if(!vertexArrayId) {
            sp<GLVertexArray> va = sp<GLVertexArray>::make(context._shader_bindings->getPipeline(graphicsContext), context._vertices.delegate(), context._shader_bindings);
            va->upload(graphicsContext);
            graphicsContext.renderController()->upload(va, RenderController::US_ON_SURFACE_READY);
            vertexArrayId = va->id();
            context._attachments->put(std::move(va));
        }
        glBindVertexArray(static_cast<GLuint>(vertexArrayId));
        if(context._indices)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(context._indices.id()));
    }

    virtual void postDraw(GraphicsContext& /*graphicsContext*/) override {
        glBindVertexArray(0);
    }

};

class SnippetGLES30 : public Snippet {
public:
    virtual void preInitialize(PipelineBuildingContext& /*context*/) override {
    }

    virtual void preCompile(GraphicsContext& /*graphicsContext*/, PipelineBuildingContext& context, const PipelineLayout& /*pipelineLayout*/) override {
        ShaderPreprocessor* vertex = context.tryGetStage(PipelineInput::SHADER_STAGE_VERTEX);
        if(vertex)
            vertex->_predefined_macros.push_back("#define gl_InstanceIndex gl_InstanceID");

        ShaderPreprocessor* fragment = context.tryGetStage(PipelineInput::SHADER_STAGE_FRAGMENT);
        if(fragment) {
            fragment->linkNextStage("FragColor");
            fragment->_predefined_macros.push_back("#define texture2D texture");
            fragment->_predefined_macros.push_back("#define textureCube texture");
        }
    }

    virtual sp<DrawEvents> makeDrawEvents() override {
        return sp<DrawEventsGLES30>::make();
    }
};

}

sp<Snippet> SnippetFactoryGLES30::createCoreSnippet()
{
    return sp<SnippetGLES30>::make();
}

}
}
