#pragma once

#include "core/base/string.h"
#include "core/inf/builder.h"

#include "renderer/inf/snippet.h"

namespace ark {

class SnippetSwizzle final : public Snippet {
public:
    SnippetSwizzle(const String& components);

    void preInitialize(PipelineBuildingContext& context) override;

//  [[plugin::builder::by-value("swizzle")]]
    class BUILDER final : public Builder<Snippet> {
    public:
        BUILDER(String components);

        sp<Snippet> build(const Scope& args) override;

    private:
        String _components;
    };

private:
    String _pre_modifier;
    String _post_modifier;
};

}
