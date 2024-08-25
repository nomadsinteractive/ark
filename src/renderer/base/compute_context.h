#pragma once

#include <array>
#include <vector>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/pipeline_context.h"

namespace ark {

class ARK_API ComputeContext : public PipelineContext {
public:
    ComputeContext(sp<PipelineBindings> pipelineBindings, sp<RenderLayerSnapshot::BufferObject> bufferObject, std::array<uint32_t, 3> numWorkGroups);
    DEFAULT_COPY_AND_ASSIGN(ComputeContext);

    sp<RenderCommand> toComputeCommand();

    std::array<uint32_t, 3> _num_work_groups;
};

}
