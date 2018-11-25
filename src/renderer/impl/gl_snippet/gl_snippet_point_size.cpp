#include "renderer/impl/gl_snippet/gl_snippet_point_size.h"

#include "renderer/base/pipeline_layout.h"

namespace ark {

void GLSnippetPointSize::preInitialize(PipelineLayout& source)
{
    source.addAttribute("PointSize", "float");
}

void GLSnippetPointSize::preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context)
{
    context._vertex.addSource("gl_PointSize = a_PointSize;");
}

sp<GLSnippet> GLSnippetPointSize::DICTIONARY::build(const sp<Scope>& /*args*/)
{
    return sp<GLSnippetPointSize>::make();
}

}
