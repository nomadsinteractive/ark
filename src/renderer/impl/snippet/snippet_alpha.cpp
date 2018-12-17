#include "renderer/impl/snippet/snippet_alpha.h"

#include "renderer/base/pipeline_layout.h"

namespace ark {

void SnippetAlpha::preInitialize(PipelineLayout& source)
{
    source.addAttribute("Alpha", "float", ShaderPreprocessor::SHADER_TYPE_FRAGMENT);
}

void SnippetAlpha::preCompile(GraphicsContext& /*graphicsContext*/, PipelineBuildingContext& context)
{
    context._fragment.addModifier("vec4(1.0, 1.0, 1.0, v_Alpha)");
}

sp<Snippet> SnippetAlpha::DICTIONARY::build(const sp<Scope>& /*args*/)
{
    return sp<SnippetAlpha>::make();
}

}
