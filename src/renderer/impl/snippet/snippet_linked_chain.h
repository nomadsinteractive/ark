#pragma once

#include "core/inf/builder.h"
#include "core/base/bean_factory.h"
#include "core/types/shared_ptr.h"

#include "renderer/inf/snippet.h"

namespace ark {

class SnippetLinkedChain : public Snippet {
public:
    SnippetLinkedChain(sp<Snippet> delegate, sp<Snippet> next);

    virtual void preInitialize(PipelineBuildingContext& context) override;
    virtual void preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout) override;
    virtual sp<DrawEvents> makeDrawEvents(const RenderRequest& renderRequest) override;
    virtual sp<DrawEvents> makeDrawEvents() override;

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Snippet> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value);

        virtual sp<Snippet> build(const Scope& args) override;

    private:
        BeanFactory _factory;
        String _value;

    };

//  [[plugin::builder]]
    class BUILDER : public Builder<Snippet> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Snippet> build(const Scope& args) override;

    private:
        sp<Snippet> _build(const document& manifest, const Scope& args);

    private:
        BeanFactory _factory;
        document _manifest;
    };

private:
    sp<Snippet::DrawEvents> makeDrawEvents(sp<Snippet::DrawEvents> de1, sp<Snippet::DrawEvents> de2) const;

private:
    sp<Snippet> _delegate;
    sp<Snippet> _next;
};

}
