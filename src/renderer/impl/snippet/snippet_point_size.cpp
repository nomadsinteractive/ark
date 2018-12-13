#include "renderer/impl/snippet/snippet_point_size.h"

#include "renderer/base/pipeline_layout.h"

namespace ark {

void SnippetPointSize::preInitialize(PipelineLayout& source)
{
    source.addAttribute("PointSize", "float");
}

void SnippetPointSize::preCompile(GraphicsContext& /*graphicsContext*/, PipelineBuildingContext& context)
{
    context._vertex.addSource("gl_PointSize = a_PointSize;");
}

sp<Snippet> SnippetPointSize::DICTIONARY::build(const sp<Scope>& /*args*/)
{
    return sp<SnippetPointSize>::make();
}

}
