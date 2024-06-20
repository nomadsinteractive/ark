#pragma once

#include "core/inf/builder.h"

#include "renderer/inf/snippet.h"

namespace ark {

class SnippetTint final : public Snippet {
public:
    void preInitialize(PipelineBuildingContext& context) override;

//  [[plugin::builder::by-value("tint")]]
    class DICTIONARY : public Builder<Snippet> {
    public:
        DICTIONARY() = default;

        sp<Snippet> build(const Scope& args) override;

    };

};

}
