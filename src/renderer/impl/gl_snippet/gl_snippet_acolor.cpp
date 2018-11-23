#include "renderer/impl/gl_snippet/gl_snippet_acolor.h"

#include "renderer/base/pipeline_layout.h"

namespace ark {

void GLSnippetAcolor::preInitialize(PipelineLayout& source)
{
    source.addPredefinedAttribute("Color", "vec4", GLShaderPreprocessor::SHADER_TYPE_FRAGMENT);
}

void GLSnippetAcolor::preCompile(GraphicsContext& /*graphicsContext*/, GLShaderPreprocessorContext& context)
{
    context.addFragmentColorModifier("v_Color");
}

sp<GLSnippet> GLSnippetAcolor::DICTIONARY::build(const sp<Scope>& args)
{
    return sp<GLSnippetAcolor>::make();
}

}
