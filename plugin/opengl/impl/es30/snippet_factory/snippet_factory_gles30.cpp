#include "opengl/impl/es30/snippet_factory/snippet_factory_gles30.h"

#include "renderer/inf/snippet.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/render_controller.h"

#include "opengl/impl/es30/gl_resource/gl_vertex_array.h"

namespace ark::plugin::opengl {

namespace {

class SnippetGLES30 final : public Snippet {
public:
    void preCompile(GraphicsContext& /*graphicsContext*/, PipelineBuildingContext& context, const PipelineConfiguration& /*pipelineLayout*/) override {
        if(ShaderPreprocessor* vertex = context.tryGetRenderStage(Enum::SHADER_STAGE_BIT_VERTEX))
            vertex->_predefined_macros.push_back("#define gl_InstanceIndex gl_InstanceID");

        if(ShaderPreprocessor* fragment = context.tryGetRenderStage(Enum::SHADER_STAGE_BIT_FRAGMENT))
            fragment->linkNextStage("FragColor");
    }
};

}

sp<Snippet> SnippetFactoryGLES30::createCoreSnippet()
{
    return sp<Snippet>::make<SnippetGLES30>();
}

}
