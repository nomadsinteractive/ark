#pragma once

#include <array>

#include "core/types/shared_ptr.h"

#include "renderer/inf/snippet.h"

namespace ark {

class SnippetDrawCompute final : public Snippet {
public:
    SnippetDrawCompute(sp<PipelineLayout> shaderLayout, std::array<uint32_t, 3> numWorkGroups, bool atPostDraw);

    sp<DrawDecorator> makeDrawDecorator(const RenderRequest& renderRequest) override;

private:
    sp<PipelineLayout> _shader_layout;
    std::array<uint32_t, 3> _num_work_groups;
    bool _at_post_draw;
};

}
