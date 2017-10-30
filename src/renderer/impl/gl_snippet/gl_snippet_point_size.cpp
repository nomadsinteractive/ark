#include "renderer/impl/gl_snippet/gl_snippet_point_size.h"

#include "renderer/base/gl_shader_source.h"

namespace ark {

void GLSnippetPointSize::preCompile(GLShaderSource& source, GLShaderPreprocessor::Context& context)
{
    source.addPredefinedAttribute("PointSize", "float");
    context.addVertexSource("gl_PointSize = a_PointSize;");
}

sp<GLSnippet> GLSnippetPointSize::DICTIONARY::build(const sp<Scope>& /*args*/)
{
    return sp<GLSnippetPointSize>::make();
}

}
