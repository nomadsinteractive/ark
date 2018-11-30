#include "renderer/impl/gl_snippet/gl_snippet_linked_chain.h"

#include "core/base/bean_factory.h"
#include "core/util/strings.h"
#include "core/util/identifier.h"

namespace ark {

GLSnippetLinkedChain::GLSnippetLinkedChain(const sp<GLSnippet>& delegate, const sp<GLSnippet>& next)
    : _delegate(delegate), _next(next)
{
    DASSERT(delegate);
    DASSERT(next);
}

void GLSnippetLinkedChain::preInitialize(PipelineLayout& source)
{
    _delegate->preInitialize(source);
    _next->preInitialize(source);
}

void GLSnippetLinkedChain::preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context)
{
    _delegate->preCompile(graphicsContext, context);
    _next->preCompile(graphicsContext, context);
}

void GLSnippetLinkedChain::preDraw(GraphicsContext& graphicsContext, const Shader& shader, const DrawingContext& context)
{
    _delegate->preDraw(graphicsContext, shader, context);
    _next->preDraw(graphicsContext, shader, context);
}

void GLSnippetLinkedChain::postDraw(GraphicsContext& graphicsContext)
{
    _delegate->postDraw(graphicsContext);
    _next->postDraw(graphicsContext);
}

GLSnippetLinkedChain::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& value)
    : _factory(factory), _value(value)
{
}

sp<GLSnippet> GLSnippetLinkedChain::DICTIONARY::build(const sp<Scope>& args)
{
    const std::vector<String> l = Strings::unwrap(_value, '[', ']').split(',');
    DCHECK(l.size() > 0, "Empty list");
    if(l.size() == 1)
    {
        const Identifier id = Identifier::parse(l.at(0));
        const sp<Builder<GLSnippet>> builder = id.isVal() ? _factory.ensureBuilderByTypeValue<GLSnippet>(id.val(), id.val()) : _factory.ensureBuilder<GLSnippet>(l.at(0));
        DCHECK(builder, "Cannot build \"%s\"", id.toString().c_str());
        return builder->build(args);
    }

    sp<GLSnippetLinkedChain> chain = sp<GLSnippetLinkedChain>::make(_factory.ensure<GLSnippet>(l.at(0), args), _factory.ensure<GLSnippet>(l.at(1), args));
    for(size_t i = 2; i < l.size(); i++)
        chain = sp<GLSnippetLinkedChain>::make(chain, _factory.ensure<GLSnippet>(l.at(i), args));
    return chain;
}

GLSnippetLinkedChain::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _factory(factory), _manifest(manifest)
{
}

sp<GLSnippet> GLSnippetLinkedChain::BUILDER::build(const sp<Scope>& args)
{
    sp<GLSnippet> snippet;
    sp<GLSnippetLinkedChain> chain;
    for(const document& i : _manifest->children(Constants::Attributes::SNIPPET))
    {
        if(!chain && snippet)
            chain = sp<GLSnippetLinkedChain>::make(snippet, _build(i, args));
        else
        {
            snippet = _build(i, args);
            if(chain)
                chain = sp<GLSnippetLinkedChain>::make(chain, snippet);
        }
    }
    DWARN(chain, "Only one snippet in SnippetLinkedChain");
    return chain ? chain.cast<GLSnippet>() : snippet;
}

sp<GLSnippet> GLSnippetLinkedChain::BUILDER::_build(const document& manifest, const sp<Scope>& args)
{
    const String type = Documents::getAttribute(manifest, Constants::Attributes::TYPE);
    const String value = Documents::getAttribute(manifest, Constants::Attributes::VALUE);
    if(type)
        return _factory.ensureBuilderByTypeValue<GLSnippet>(type, value)->build(args);
    return _factory.ensure<GLSnippet>(manifest, args);
}

}
