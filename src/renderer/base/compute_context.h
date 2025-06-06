#pragma once

#include <array>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/pipeline_context.h"

namespace ark {

class ARK_API ComputeContext : public PipelineContext {
public:
    ComputeContext(sp<PipelineBindings> pipelineBindings, sp<RenderBufferSnapshot> bufferSnapshot, V3i numWorkGroups);
    DEFAULT_COPY_AND_ASSIGN(ComputeContext);

    sp<RenderCommand> toComputeCommand();

    V3i _num_work_groups;
};

}
