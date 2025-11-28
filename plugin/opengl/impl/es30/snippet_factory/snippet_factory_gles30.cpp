#include "opengl/impl/es30/snippet_factory/snippet_factory_gles30.h"

#include <ranges>

#include "renderer/inf/snippet.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/render_controller.h"

#include "opengl/impl/es30/gl_resource/gl_vertex_array.h"

namespace ark::plugin::opengl {

namespace {

class SnippetGLES30 final : public Snippet {
public:
    void preCompile(PipelineBuildingContext& context) override
    {
        for(const auto& renderStage : context.renderStages() | std::views::values)
            renderStage->_predefined_macros.emplace_back("#define ARK_USE_OPENGL");
        if(const auto& computeStage = context.computingStage())
            computeStage->_predefined_macros.emplace_back("#define ARK_USE_OPENGL");

        if(ShaderPreprocessor* vertex = context.tryGetRenderStage(enums::SHADER_STAGE_BIT_VERTEX))
            vertex->_predefined_macros.emplace_back("#define gl_InstanceIndex (gl_BaseInstance + gl_InstanceID)");

        if(ShaderPreprocessor* fragment = context.tryGetRenderStage(enums::SHADER_STAGE_BIT_FRAGMENT))
            fragment->linkNextStage("FragColor");
    }
};

}

sp<Snippet> SnippetFactoryGLES30::createCoreSnippet()
{
    return sp<Snippet>::make<SnippetGLES30>();
}

}
