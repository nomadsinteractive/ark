#include "renderer/impl/gl_snippet/gl_snippet_point_size.h"

#include "renderer/base/pipeline_layout.h"

namespace ark {

void GLSnippetPointSize::preInitialize(PipelineLayout& source)
{
    source.addPredefinedAttribute("PointSize", "float");
}

void GLSnippetPointSize::preCompile(GraphicsContext& graphicsContext, GLShaderPreprocessorContext& context)
{
    context.addVertexSource("gl_PointSize = a_PointSize;");
}

sp<GLSnippet> GLSnippetPointSize::DICTIONARY::build(const sp<Scope>& /*args*/)
{
    return sp<GLSnippetPointSize>::make();
}

}
