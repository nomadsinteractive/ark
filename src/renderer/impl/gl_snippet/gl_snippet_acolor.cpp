#include "renderer/impl/gl_snippet/gl_snippet_acolor.h"

#include "renderer/base/gl_shader_source.h"

namespace ark {

void GLSnippetAcolor::preInitialize(GLShaderSource& source)
{
    source.addPredefinedAttribute("Color", "vec4", GLShaderPreprocessor::SHADER_TYPE_FRAGMENT);
}

void GLSnippetAcolor::preCompile(GraphicsContext& /*graphicsContext*/, GLShaderPreprocessor::Context& context)
{
    context.addFragmentColorModifier("v_Color");
}

sp<GLSnippet> GLSnippetAcolor::DICTIONARY::build(const sp<Scope>& args)
{
    return sp<GLSnippetAcolor>::make();
}

}
