#include "renderer/opengl/es30/snippet_factory/snippet_factory_gles30.h"

#include "renderer/inf/snippet.h"

#include "renderer/base/resource_manager.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/pipeline_building_context.h"

#include "renderer/opengl/es30/gl_resource/gl_vertex_array.h"

#include "platform/gl/gl.h"

namespace ark {
namespace gles30 {

namespace {

class SnippetGLES30 : public Snippet {
public:
    SnippetGLES30(const sp<Resource>& vertexArray)
        : _vertex_array(vertexArray) {
    }

    virtual void preCompile(GraphicsContext& /*graphicsContext*/, PipelineBuildingContext& context, const sp<ShaderBindings>& /*shaderBindings*/) override {
        context._fragment._outs.declare("vec4", "v_", "FragColor");
        context._fragment._macro_defines.push_back("#define texture2D texture");
        context._fragment._macro_defines.push_back("#define textureCube texture");
    }

    virtual void preDraw(GraphicsContext& /*graphicsContext*/, Shader& /*shader*/, const DrawingContext& context) override {
        glBindVertexArray(_vertex_array->id());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context._index_buffer.id());
    }

    virtual void postDraw(GraphicsContext& /*graphicsContext*/) override {
        glBindVertexArray(0);
    }

private:
    sp<Resource> _vertex_array;
};

}

sp<Snippet> SnippetFactoryGLES30::createCoreSnippet(ResourceManager& resourceManager, const sp<PipelineFactory>& pipelineFactory, const sp<ShaderBindings>& shaderBindings)
{
    return sp<SnippetGLES30>::make(resourceManager.createGLResource<GLVertexArray>(pipelineFactory, shaderBindings));
}

}
}
