#include "renderer/impl/snippet/snippet_linked_chain.h"

#include "core/base/bean_factory.h"
#include "core/util/strings.h"
#include "core/util/identifier.h"

namespace ark {

SnippetLinkedChain::SnippetLinkedChain(const sp<Snippet>& delegate, const sp<Snippet>& next)
    : _delegate(delegate), _next(next)
{
    DASSERT(delegate);
    DASSERT(next);
}

void SnippetLinkedChain::preInitialize(PipelineBuildingContext& context)
{
    _delegate->preInitialize(context);
    _next->preInitialize(context);
}

void SnippetLinkedChain::preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const sp<ShaderBindings>& shaderBindings)
{
    _delegate->preCompile(graphicsContext, context, shaderBindings);
    _next->preCompile(graphicsContext, context, shaderBindings);
}

void SnippetLinkedChain::preDraw(GraphicsContext& graphicsContext, const DrawingContext& context)
{
    _delegate->preDraw(graphicsContext, context);
    _next->preDraw(graphicsContext, context);
}

void SnippetLinkedChain::postDraw(GraphicsContext& graphicsContext)
{
    _delegate->postDraw(graphicsContext);
    _next->postDraw(graphicsContext);
}

SnippetLinkedChain::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& value)
    : _factory(factory), _value(value)
{
}

sp<Snippet> SnippetLinkedChain::DICTIONARY::build(const sp<Scope>& args)
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

sp<Snippet> SnippetLinkedChain::BUILDER::build(const sp<Scope>& args)
{
    sp<Snippet> snippet;
    sp<SnippetLinkedChain> chain;
    for(const document& i : _manifest->children(Constants::Attributes::SNIPPET))
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
    DWARN(chain, "Only one snippet in SnippetLinkedChain");
    return chain ? chain.cast<Snippet>() : snippet;
}

sp<Snippet> SnippetLinkedChain::BUILDER::_build(const document& manifest, const sp<Scope>& args)
{
    const String type = Documents::getAttribute(manifest, Constants::Attributes::TYPE);
    const String value = Documents::getAttribute(manifest, Constants::Attributes::VALUE);
    if(type)
        return _factory.ensureBuilderByTypeValue<Snippet>(type, value)->build(args);
    return _factory.ensure<Snippet>(manifest, args);
}

}
