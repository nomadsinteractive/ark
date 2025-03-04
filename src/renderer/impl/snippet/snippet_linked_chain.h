#pragma once

#include "core/inf/builder.h"
#include "core/base/bean_factory.h"
#include "core/types/shared_ptr.h"

#include "renderer/inf/snippet.h"

namespace ark {

class SnippetLinkedChain final : public Snippet {
public:
    SnippetLinkedChain(sp<Snippet> delegate, sp<Snippet> next);

    void preInitialize(PipelineBuildingContext& context) override;
    void preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout) override;
    sp<DrawDecorator> makeDrawDecorator(const RenderRequest& renderRequest) override;
    sp<DrawDecorator> makeDrawDecorator() override;

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Snippet> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value);

        sp<Snippet> build(const Scope& args) override;

    private:
        BeanFactory _factory;
        String _value;

    };

//  [[plugin::builder]]
    class BUILDER : public Builder<Snippet> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Snippet> build(const Scope& args) override;

    private:
        sp<Snippet> _build(const document& manifest, const Scope& args);

    private:
        BeanFactory _factory;
        document _manifest;
    };

private:
    sp<Snippet::DrawDecorator> makeDrawEvents(sp<Snippet::DrawDecorator> de1, sp<Snippet::DrawDecorator> de2) const;

private:
    sp<Snippet> _delegate;
    sp<Snippet> _next;
};

}
