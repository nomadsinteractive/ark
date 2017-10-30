#include "renderer/impl/gl_snippet/gl_snippet_alpha.h"

#include "renderer/base/gl_shader_source.h"

namespace ark {

void GLSnippetAlpha::preCompile(GLShaderSource& source, GLShaderPreprocessor::Context& context)
{
    source.addPredefinedAttribute("Alpha", "float");
    context.addFragmentColorModifier("vec4(1.0, 1.0, 1.0, v_Alpha)");
    context.fragmentIns.push_back(std::pair<String, String>("float", "Alpha"));
}

sp<GLSnippet> GLSnippetAlpha::DICTIONARY::build(const sp<Scope>& args)
{
    return sp<GLSnippetAlpha>::make();
}

}
