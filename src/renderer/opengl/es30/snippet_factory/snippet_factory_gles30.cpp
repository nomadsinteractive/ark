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

class SnippetGLES30 : public Snippet {
public:
    SnippetGLES30(const sp<PipelineFactory>& pipelineFactory)
        : _pipeline_factory(pipelineFactory) {
    }

    virtual void preCompile(GraphicsContext& /*graphicsContext*/, PipelineBuildingContext& context, const PipelineLayout& /*pipelineLayout*/) override {
        context._fragment._outs.declare("vec4", "v_", "FragColor");
        context._fragment._macro_defines.push_back("#define texture2D texture");
        context._fragment._macro_defines.push_back("#define textureCube texture");
    }

    virtual void preDraw(GraphicsContext& graphicsContext, Shader& /*shader*/, const DrawingContext& context) override {
        const sp<ShaderBindings>& bindings = context._shader_bindings;
        sp<GLVertexArray> vertexArray = bindings->attachments().get<GLVertexArray>();
        if(!vertexArray) {
            vertexArray = sp<GLVertexArray>::make(_pipeline_factory, bindings);
            vertexArray->upload(graphicsContext, nullptr);
            bindings->attachments().put(vertexArray);
            graphicsContext.renderController()->upload(vertexArray, nullptr, RenderController::US_ON_SURFACE_READY);
        }
        glBindVertexArray(static_cast<GLuint>(vertexArray->id()));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(context._index_buffer.id()));
    }

    virtual void postDraw(GraphicsContext& /*graphicsContext*/) override {
        glBindVertexArray(0);
    }

private:
    sp<PipelineFactory> _pipeline_factory;

};

}

sp<Snippet> SnippetFactoryGLES30::createCoreSnippet(RenderController& renderController, const sp<PipelineFactory>& pipelineFactory)
{
    return sp<SnippetGLES30>::make(pipelineFactory);
}

}
}
