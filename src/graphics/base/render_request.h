#pragma once

#include <atomic>

#include "core/base/api.h"
#include "core/base/allocator.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API RenderRequest {
public:
    RenderRequest() = default;
    RenderRequest(uint32_t tick, sp<Allocator::Pool> allocatorPool);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(RenderRequest);

    uint32_t tick() const;
    Allocator& allocator() const;

    void onRenderFrame(V4 backgroundColor, RenderView& renderView) const;

    void addRenderCommand(sp<RenderCommand> renderCommand) const;

public:
    struct Stub {
        Stub(uint32_t tick, sp<Allocator::Pool> allocatorPool);

        uint32_t _tick;
        Allocator _allocator;

        sp<RenderCommandPipeline> _render_command_pipe_line;
    };

private:
    RenderRequest(sp<Stub> stub);

private:
    sp<Stub> _stub;

    friend struct Stub;
};

}
