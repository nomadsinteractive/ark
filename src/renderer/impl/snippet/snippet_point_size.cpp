#include "renderer/impl/snippet/snippet_point_size.h"

#include "renderer/base/pipeline_building_context.h"

namespace ark {

void SnippetPointSize::preInitialize(PipelineBuildingContext& context)
{
    context.addPredefinedAttribute("PointSize", "float", 0);
    context._vertex.addPreMainSource("gl_PointSize = a_PointSize;");
}

sp<Snippet> SnippetPointSize::DICTIONARY::build(const sp<Scope>& /*args*/)
{
    return sp<SnippetPointSize>::make();
}

}
