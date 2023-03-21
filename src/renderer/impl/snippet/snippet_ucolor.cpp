#include "renderer/impl/snippet/snippet_ucolor.h"

#include "core/impl/uploader/input_variable.h"

#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/uniform.h"

namespace ark {

SnippetUColor::SnippetUColor(sp<Vec4> color)
    : _color(std::move(color))
{
}

void SnippetUColor::preInitialize(PipelineBuildingContext& context)
{
    ShaderPreprocessor& fragment = context.getStage(PipelineInput::SHADER_STAGE_FRAGMENT);
    context.addUniform("u_Color", Uniform::TYPE_F4, 1, sp<InputVariable<V4>>::make(_color), -1);
    fragment.addOutputVarModifier("u_Color");
}

SnippetUColor::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& value)
    : _color(factory.ensureBuilder<Vec4>(value))
{
}

sp<Snippet> SnippetUColor::DICTIONARY::build(const Scope& args)
{
    return sp<SnippetUColor>::make(_color->build(args));
}

}
