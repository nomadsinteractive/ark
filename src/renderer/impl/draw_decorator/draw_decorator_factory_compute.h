#pragma once

#include <array>

#include "core/types/shared_ptr.h"

#include "renderer/inf/draw_decorator.h"

namespace ark {

class DrawDecoratorFactoryCompute final : public DrawDecoratorFactory {
public:
    DrawDecoratorFactoryCompute(sp<PipelineLayout> pipelineLayout, std::array<uint32_t, 3> numWorkGroups, bool atPostDraw);

    sp<DrawDecorator> makeDrawDecorator(const RenderRequest& renderRequest) override;

private:
    sp<PipelineLayout> _pipeline_layout;
    std::array<uint32_t, 3> _num_work_groups;
    bool _at_post_draw;
};

}
