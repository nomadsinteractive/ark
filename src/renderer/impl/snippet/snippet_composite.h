#pragma once

#include "core/inf/builder.h"
#include "core/base/bean_factory.h"
#include "core/types/shared_ptr.h"

#include "renderer/inf/snippet.h"

namespace ark {

class SnippetComposite final : public Snippet {
public:
    SnippetComposite(sp<Snippet> delegate, sp<Snippet> next);

    void preInitialize(PipelineBuildingContext& context) override;
    void preCompile(PipelineBuildingContext& context) override;

    static sp<Snippet> compose(sp<Snippet> self, sp<Snippet> next);

//  [[plugin::builder::by-value]]
    class DICTIONARY final : public Builder<Snippet> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value);

        sp<Snippet> build(const Scope& args) override;

    private:
        BeanFactory _factory;
        String _value;

    };

//  [[plugin::builder]]
    class BUILDER final : public Builder<Snippet> {
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
    sp<Snippet> _delegate;
    sp<Snippet> _next;
};

}
