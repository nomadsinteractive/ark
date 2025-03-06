#include "renderer/impl/snippet/snippet_composite.h"

#include "core/base/bean_factory.h"
#include "core/util/strings.h"
#include "core/base/identifier.h"

#include "renderer/impl/snippet/draw_decorator_composite.h"

namespace ark {

SnippetComposite::SnippetComposite(sp<Snippet> delegate, sp<Snippet> next)
    : _delegate(std::move(delegate)), _next(std::move(next))
{
    DASSERT(_delegate);
    DASSERT(_next);
}

void SnippetComposite::preInitialize(PipelineBuildingContext& context)
{
    _delegate->preInitialize(context);
    _next->preInitialize(context);
}

void SnippetComposite::preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const PipelineConfiguration& pipelineLayout)
{
    _delegate->preCompile(graphicsContext, context, pipelineLayout);
    _next->preCompile(graphicsContext, context, pipelineLayout);
}

sp<DrawDecorator> SnippetComposite::makeDrawDecorator(const RenderRequest& renderRequest)
{
    return DrawDecoratorComposite::compose(_delegate->makeDrawDecorator(renderRequest), _next->makeDrawDecorator(renderRequest));
}

SnippetComposite::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& value)
    : _factory(factory), _value(value)
{
}

sp<Snippet> SnippetComposite::DICTIONARY::build(const Scope& args)
{
    const Vector<String> l = Strings::unwrap(_value, '[', ']').split(',');
    DCHECK(l.size() > 0, "Empty list");
    if(l.size() == 1)
    {
        const Identifier id = Identifier::parse(l.at(0));
        const sp<Builder<Snippet>> builder = id.isVal() ? _factory.ensureBuilderByTypeValue<Snippet>(id.val(), id.val()) : _factory.ensureBuilder<Snippet>(l.at(0));
        DCHECK(builder, "Cannot build \"%s\"", id.toString().c_str());
        return builder->build(args);
    }

    sp<Snippet> chain = sp<Snippet>::make<SnippetComposite>(_factory.ensure<Snippet>(l.at(0), args), _factory.ensure<Snippet>(l.at(1), args));
    for(size_t i = 2; i < l.size(); i++)
        chain = sp<Snippet>::make<SnippetComposite>(std::move(chain), _factory.ensure<Snippet>(l.at(i), args));
    return chain;
}

SnippetComposite::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _factory(factory), _manifest(manifest)
{
}

sp<Snippet> SnippetComposite::BUILDER::build(const Scope& args)
{
    sp<Snippet> snippet;
    sp<Snippet> next;
    for(const document& i : _manifest->children(constants::SNIPPET))
    {
        if(!next && snippet)
            next = sp<Snippet>::make<SnippetComposite>(std::move(snippet), _build(i, args));
        else
        {
            snippet = _build(i, args);
            if(next)
                next = sp<Snippet>::make<SnippetComposite>(std::move(next), snippet);
        }
    }
    DCHECK_WARN(next, "Only one snippet in SnippetLinkedChain");
    return std::move(next ? next : snippet);
}

sp<Snippet> SnippetComposite::BUILDER::_build(const document& manifest, const Scope& args)
{
    const String type = Documents::getAttribute(manifest, constants::TYPE);
    const String& value = Documents::ensureAttribute(manifest, constants::VALUE);
    if(type)
        return _factory.ensureBuilderByTypeValue<Snippet>(type, value)->build(args);
    return _factory.ensure<Snippet>(manifest, args);
}

}
