#pragma once

#include <array>
#include <vector>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/pipeline_snapshot.h"

namespace ark {

class ARK_API ComputeContext {
public:
    ComputeContext(PipelineSnapshot pipelineContext, std::array<int32_t, 3> numWorkGroups);
    DEFAULT_COPY_AND_ASSIGN(ComputeContext);

    sp<RenderCommand> toComputeCommand();

    PipelineSnapshot _pipeline_context;
    std::array<int32_t, 3> _num_work_groups;
};

}
