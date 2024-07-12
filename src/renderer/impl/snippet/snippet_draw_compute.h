#pragma once

#include <array>

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/inf/snippet.h"

namespace ark {

class SnippetDrawCompute final : public Snippet {
public:
    SnippetDrawCompute(sp<Shader> shader, std::array<sp<Integer>, 3> numWorkGroups, bool atPostDraw);

    sp<DrawEvents> makeDrawEvents(const RenderRequest& renderRequest) override;

//  [[plugin::builder("compute")]]
    class BUILDER final : public Builder<Snippet> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Snippet> build(const Scope& args) override;

    private:
        sp<Builder<Shader>> _shader;
        std::array<sp<Builder<Integer>>, 3> _num_work_groups;
        bool _at_post_draw;
    };

private:
    sp<Shader> _shader;
    std::array<sp<Integer>, 3> _num_work_groups;
    sp<ShaderBindings> _shader_bindings;
    bool _at_post_draw;
};

}
