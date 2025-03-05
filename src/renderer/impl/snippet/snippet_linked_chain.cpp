#include "renderer/impl/snippet/snippet_linked_chain.h"

#include "core/base/bean_factory.h"
#include "core/util/strings.h"
#include "core/base/identifier.h"

namespace ark {

namespace {

class DrawDecoratorComposite final : public Snippet::DrawDecorator {
public:
    DrawDecoratorComposite(sp<DrawDecorator> delegate, sp<DrawDecorator> next)
        : _delegate(std::move(delegate)), _next(std::move(next)) {
        DASSERT(_delegate && _next);
    }

    void preDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override {
        _delegate->preDraw(graphicsContext, context);
        _next->preDraw(graphicsContext, context);
    }

    void postDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override {
        _next->postDraw(graphicsContext, context);
        _delegate->postDraw(graphicsContext, context);
    }

private:
    sp<DrawDecorator> _delegate;
    sp<DrawDecorator> _next;
};


sp<Snippet::DrawDecorator> makeDrawDecoratorComposite(sp<Snippet::DrawDecorator> de1, sp<Snippet::DrawDecorator> de2)
{
    if(de1 && de2)
        return sp<Snippet::DrawDecorator>::make<DrawDecoratorComposite>(std::move(de1), std::move(de2));
    return std::move(de1 ? de1 : de2);
}

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

sp<Snippet::DrawDecorator> SnippetLinkedChain::makeDrawDecorator(const RenderRequest& renderRequest)
{
    return makeDrawDecoratorComposite(_delegate->makeDrawDecorator(renderRequest), _next->makeDrawDecorator(renderRequest));
}

sp<Snippet::DrawDecorator> SnippetLinkedChain::makeDrawDecorator()
{
    return makeDrawDecoratorComposite(_delegate->makeDrawDecorator(), _next->makeDrawDecorator());
}

SnippetLinkedChain::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& value)
    : _factory(factory), _value(value)
{
}

sp<Snippet> SnippetLinkedChain::DICTIONARY::build(const Scope& args)
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

    sp<Snippet> chain = sp<Snippet>::make<SnippetLinkedChain>(_factory.ensure<Snippet>(l.at(0), args), _factory.ensure<Snippet>(l.at(1), args));
    for(size_t i = 2; i < l.size(); i++)
        chain = sp<Snippet>::make<SnippetLinkedChain>(std::move(chain), _factory.ensure<Snippet>(l.at(i), args));
    return chain;
}

SnippetLinkedChain::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _factory(factory), _manifest(manifest)
{
}

sp<Snippet> SnippetLinkedChain::BUILDER::build(const Scope& args)
{
    sp<Snippet> snippet;
    sp<Snippet> next;
    for(const document& i : _manifest->children(constants::SNIPPET))
    {
        if(!next && snippet)
            next = sp<Snippet>::make<SnippetLinkedChain>(std::move(snippet), _build(i, args));
        else
        {
            snippet = _build(i, args);
            if(next)
                next = sp<Snippet>::make<SnippetLinkedChain>(std::move(next), snippet);
        }
    }
    DCHECK_WARN(next, "Only one snippet in SnippetLinkedChain");
    return std::move(next ? next : snippet);
}

sp<Snippet> SnippetLinkedChain::BUILDER::_build(const document& manifest, const Scope& args)
{
    const String type = Documents::getAttribute(manifest, constants::TYPE);
    const String& value = Documents::ensureAttribute(manifest, constants::VALUE);
    if(type)
        return _factory.ensureBuilderByTypeValue<Snippet>(type, value)->build(args);
    return _factory.ensure<Snippet>(manifest, args);
}

}
