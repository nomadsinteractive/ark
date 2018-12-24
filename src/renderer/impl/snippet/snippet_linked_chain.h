#ifndef ARK_RENDERER_IMPL_SNIPPET_LINKED_CHAIN_H_
#define ARK_RENDERER_IMPL_SNIPPET_LINKED_CHAIN_H_

#include "core/inf/builder.h"
#include "core/base/bean_factory.h"
#include "core/types/shared_ptr.h"

#include "renderer/inf/snippet.h"

namespace ark {

class SnippetLinkedChain : public Snippet {
public:
    SnippetLinkedChain(const sp<Snippet>& delegate, const sp<Snippet>& next);

    virtual void preInitialize(PipelineBuildingContext& context) override;
    virtual void preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context) override;
    virtual void preDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override;
    virtual void postDraw(GraphicsContext& graphicsContext) override;

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Snippet> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value);

        virtual sp<Snippet> build(const sp<Scope>& args) override;

    private:
        BeanFactory _factory;
        String _value;

    };

//  [[plugin::builder]]
    class BUILDER : public Builder<Snippet> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Snippet> build(const sp<Scope>& args) override;

    private:
        sp<Snippet> _build(const document& manifest, const sp<Scope>& args);

    private:
        BeanFactory _factory;
        document _manifest;
    };

private:
    sp<Snippet> _delegate;
    sp<Snippet> _next;

};

}

#endif
