#include "renderer/impl/snippet/snippet_point_size.h"

#include "renderer/base/pipeline_building_context.h"

namespace ark {

void SnippetPointSize::preInitialize(PipelineBuildingContext& context)
{
    ShaderPreprocessor& vertex = context.getStage(PipelineInput::SHADER_STAGE_VERTEX);
    context.addPredefinedAttribute("PointSize", "float", 0, PipelineInput::SHADER_STAGE_VERTEX);
    vertex.addPreMainSource("gl_PointSize = a_PointSize;");
}

sp<Snippet> SnippetPointSize::DICTIONARY::build(const Scope& /*args*/)
{
    return sp<SnippetPointSize>::make();
}

}
