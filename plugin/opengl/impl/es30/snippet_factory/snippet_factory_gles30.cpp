#include "opengl/impl/es30/snippet_factory/snippet_factory_gles30.h"

#include <ranges>

#include "renderer/inf/snippet.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/util/render_util.h"

namespace ark::plugin::opengl {

namespace {

class SnippetGLES30 final : public Snippet {
public:
    void postInitialize(PipelineBuildingContext& context) override
    {
        for(const auto& renderStage : context.renderStages() | std::views::values)
            renderStage->_predefined_macros.emplace_back("#define ARK_USE_OPENGL");
        if(const auto& computeStage = context.computingStage())
            computeStage->_predefined_macros.emplace_back("#define ARK_USE_OPENGL");

        if(ShaderPreprocessor* vertex = context.tryGetRenderStage(enums::SHADER_STAGE_BIT_VERTEX))
            vertex->_predefined_macros.emplace_back("#define gl_InstanceIndex (gl_BaseInstance + gl_InstanceID)");

        ShaderPreprocessor* fragment = context.tryGetRenderStage(enums::SHADER_STAGE_BIT_FRAGMENT);
        ShaderPreprocessor* compute = context.tryGetRenderStage(enums::SHADER_STAGE_BIT_COMPUTE);
        if(fragment)
            fragment->linkNextStage("FragColor");

        int32_t binding = fragment ? fragment->_declaration_samplers.vars().size() : (compute ? compute->_declaration_samplers.vars().size() : 0);
        for(ShaderPreprocessor* preprocessor : context.stages())
        {
            for(const auto& [k, v] : preprocessor->_declaration_images.vars())
                RenderUtil::overrideLayoutDescriptor(k, v._source, v._binding, binding++, -1, "Image", "shaders");

            for(auto& [k, v] : preprocessor->_ssbos)
                RenderUtil::overrideLayoutDescriptor(k, v._declaration, v._binding, binding++, -1, "Storage buffer", "shaders");
        }
    }
};

}

sp<Snippet> SnippetFactoryGLES30::createCoreSnippet()
{
    return sp<Snippet>::make<SnippetGLES30>();
}

}
