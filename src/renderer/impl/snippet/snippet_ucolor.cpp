#include "renderer/impl/snippet/snippet_ucolor.h"

#include "graphics/impl/flatable/flatable_v4f.h"

#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/uniform.h"

namespace ark {

SnippetUColor::SnippetUColor(const sp<Vec4>& color)
    : _color(color)
{
}

void SnippetUColor::preInitialize(PipelineBuildingContext& context)
{
    context.addUniform("u_Color", Uniform::TYPE_F4, sp<FlatableV4f>::make(_color), _color.as<Notifier>(), -1);
    context._fragment.addModifier("u_Color");
}

SnippetUColor::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& value)
    : _color(factory.ensureBuilder<Vec4>(value))
{
}

sp<Snippet> SnippetUColor::DICTIONARY::build(const sp<Scope>& args)
{
    return sp<SnippetUColor>::make(_color->build(args));
}

}
