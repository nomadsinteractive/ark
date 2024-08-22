#include "renderer/impl/snippet/snippet_swizzle.h"

#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/pipeline_input.h"

namespace ark {

SnippetSwizzle::SnippetSwizzle(const String& components)
{
    StringBuffer pre;
    StringBuffer post;
    bool swizzled = false;
    std::vector<String> cmps = components.split(',');
    ASSERT(!cmps.empty());
    for(size_t i = 0; i < cmps.size(); ++i)
    {
        String c = cmps.at(i).strip();
        const bool isSwizzling = c.str().find_first_not_of("rgba") == std::string::npos;
        CHECK(Strings::isNumeric(c) || isSwizzling, "Invalid component \"%s\". It's neither numeric or rgba value.", c.c_str());
        if(isSwizzling)
        {
            post << '.';
            CHECK(!swizzled, "Doesn't support multiple time swizzles: %s", components.c_str());
            swizzled = true;
        }
        (swizzled ? post : pre) << std::move(c);
        if(i != cmps.size() - 1)
            (swizzled ? post : pre) << ", ";
    }
    if(cmps.size() == 1)
        _post_modifier = post.str();
    else
    {
        _pre_modifier = "vec4(" + pre.str();
        post << ')';
        _post_modifier = post.str();
    }
}

void SnippetSwizzle::preInitialize(PipelineBuildingContext& context)
{
    ShaderPreprocessor& fragment = context.getStage(Enum::SHADER_STAGE_BIT_FRAGMENT);
    fragment.addOutputModifier(_pre_modifier, _post_modifier);
}

SnippetSwizzle::BUILDER::BUILDER(String components)
    : _components(std::move(components))
{
}

sp<Snippet> SnippetSwizzle::BUILDER::build(const Scope& args)
{
    return sp<Snippet>::make<SnippetSwizzle>(_components);
}

}
