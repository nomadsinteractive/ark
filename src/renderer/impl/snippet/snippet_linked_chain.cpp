#include "renderer/impl/snippet/snippet_linked_chain.h"

#include "core/base/bean_factory.h"
#include "core/util/strings.h"
#include "core/base/identifier.h"

namespace ark {

namespace {

class DrawEventsLinkedChain final : public Snippet::DrawEvents {
public:
    DrawEventsLinkedChain(sp<Snippet::DrawEvents> delegate, sp<Snippet::DrawEvents> next)
        : _delegate(std::move(delegate)), _next(std::move(next)) {
        DASSERT(_delegate && _next);
    }

    void preDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override {
        _delegate->preDraw(graphicsContext, context);
        _next->preDraw(graphicsContext, context);
    }

    void postDraw(GraphicsContext& graphicsContext) override {
        _delegate->postDraw(graphicsContext);
        _next->postDraw(graphicsContext);
    }

private:
    sp<DrawEvents> _delegate;
    sp<DrawEvents> _next;
};

}

SnippetLinkedChain::SnippetLinkedChain(sp<Snippet> delegate, sp<Snippet> next)
    : _delegate(std::move(delegate)), _next(std::move(next))
{
    DASSERT(_delegate);
    DASSERT(_next);
}

void SnippetLinkedChain::preInitialize(PipelineBuildingContext& context)
{
    _delegate->preInitialize(context);
    _next->preInitialize(context);
}

void SnippetLinkedChain::preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout)
{
    _delegate->preCompile(graphicsContext, context, pipelineLayout);
    _next->preCompile(graphicsContext, context, pipelineLayout);
}

sp<Snippet::DrawEvents> SnippetLinkedChain::makeDrawEvents(const RenderRequest& renderRequest)
{
    return makeDrawEvents(_delegate->makeDrawEvents(renderRequest), _next->makeDrawEvents(renderRequest));
}

sp<Snippet::DrawEvents> SnippetLinkedChain::makeDrawEvents()
{
    return makeDrawEvents(_delegate->makeDrawEvents(), _next->makeDrawEvents());
}

sp<Snippet::DrawEvents> SnippetLinkedChain::makeDrawEvents(sp<Snippet::DrawEvents> de1, sp<Snippet::DrawEvents> de2) const
{
    if(de1 && de2)
        return sp<DrawEventsLinkedChain>::make(std::move(de1), std::move(de2));
    return de1 ? de1 : de2;
}

SnippetLinkedChain::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& value)
    : _factory(factory), _value(value)
{
}

sp<Snippet> SnippetLinkedChain::DICTIONARY::build(const Scope& args)
{
    const std::vector<String> l = Strings::unwrap(_value, '[', ']').split(',');
    DCHECK(l.size() > 0, "Empty list");
    if(l.size() == 1)
    {
        const Identifier id = Identifier::parse(l.at(0));
        const sp<Builder<Snippet>> builder = id.isVal() ? _factory.ensureBuilderByTypeValue<Snippet>(id.val(), id.val()) : _factory.ensureBuilder<Snippet>(l.at(0));
        DCHECK(builder, "Cannot build \"%s\"", id.toString().c_str());
        return builder->build(args);
    }

    sp<SnippetLinkedChain> chain = sp<SnippetLinkedChain>::make(_factory.ensure<Snippet>(l.at(0), args), _factory.ensure<Snippet>(l.at(1), args));
    for(size_t i = 2; i < l.size(); i++)
        chain = sp<SnippetLinkedChain>::make(chain, _factory.ensure<Snippet>(l.at(i), args));
    return chain;
}

SnippetLinkedChain::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _factory(factory), _manifest(manifest)
{
}

sp<Snippet> SnippetLinkedChain::BUILDER::build(const Scope& args)
{
    sp<Snippet> snippet;
    sp<SnippetLinkedChain> chain;
    for(const document& i : _manifest->children(constants::SNIPPET))
    {
        if(!chain && snippet)
            chain = sp<SnippetLinkedChain>::make(snippet, _build(i, args));
        else
        {
            snippet = _build(i, args);
            if(chain)
                chain = sp<SnippetLinkedChain>::make(chain, snippet);
        }
    }
    DCHECK_WARN(chain, "Only one snippet in SnippetLinkedChain");
    return chain ? chain.cast<Snippet>() : snippet;
}

sp<Snippet> SnippetLinkedChain::BUILDER::_build(const document& manifest, const Scope& args)
{
    const String type = Documents::getAttribute(manifest, constants::TYPE);
    const String value = Documents::getAttribute(manifest, constants::VALUE);
    if(type)
        return _factory.ensureBuilderByTypeValue<Snippet>(type, value)->build(args);
    return _factory.ensure<Snippet>(manifest, args);
}

}
