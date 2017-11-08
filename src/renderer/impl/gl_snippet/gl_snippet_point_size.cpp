#include "renderer/impl/gl_snippet/gl_snippet_point_size.h"

#include "renderer/base/gl_shader_source.h"

namespace ark {

void GLSnippetPointSize::preInitialize(GLShaderSource& source)
{
    source.addPredefinedAttribute("PointSize", "float");
}

void GLSnippetPointSize::preCompile(GraphicsContext& graphicsContext, GLShaderPreprocessor::Context& context)
{
    context.addVertexSource("gl_PointSize = a_PointSize;");
}

sp<GLSnippet> GLSnippetPointSize::DICTIONARY::build(const sp<Scope>& /*args*/)
{
    return sp<GLSnippetPointSize>::make();
}

}
