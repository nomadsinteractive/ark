#include "renderer/impl/gl_snippet/gl_snippet_alpha.h"

#include "renderer/base/pipeline_layout.h"

namespace ark {

void GLSnippetAlpha::preInitialize(PipelineLayout& source)
{
    source.addAttribute("Alpha", "float", GLShaderPreprocessor::SHADER_TYPE_FRAGMENT);
}

void GLSnippetAlpha::preCompile(GraphicsContext& /*graphicsContext*/, PipelineBuildingContext& context)
{
    context._fragment.addModifier("vec4(1.0, 1.0, 1.0, v_Alpha)");
}

sp<GLSnippet> GLSnippetAlpha::DICTIONARY::build(const sp<Scope>& /*args*/)
{
    return sp<GLSnippetAlpha>::make();
}

}
