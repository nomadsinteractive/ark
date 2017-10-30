#include "renderer/impl/gl_snippet/gl_snippet_acolor.h"

#include "renderer/base/gl_shader_source.h"

namespace ark {

void GLSnippetAcolor::preCompile(GLShaderSource& source, GLShaderPreprocessor::Context& context)
{
    source.addPredefinedAttribute("Color", "vec4");
    context.addFragmentColorModifier("v_Color");
    context.fragmentIns.push_back(std::pair<String, String>("vec4", "Color"));
}

sp<GLSnippet> GLSnippetAcolor::DICTIONARY::build(const sp<Scope>& args)
{
    return sp<GLSnippetAcolor>::make();
}

}
