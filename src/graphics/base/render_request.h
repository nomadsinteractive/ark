#ifndef ARK_GRAPHICS_BASE_RENDER_REQUEST_H_
#define ARK_GRAPHICS_BASE_RENDER_REQUEST_H_

#include <atomic>

#include "core/base/api.h"
#include "core/base/allocator.h"
#include "core/concurrent/one_consumer_synchronized.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API RenderRequest {
public:
    RenderRequest() = default;
    RenderRequest(uint64_t timestamp, const sp<Allocator::Pool>& allocatorPool, const sp<Executor>& executor, const sp<OCSQueue<RenderRequest>>& renderRequests);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(RenderRequest);

    uint64_t timestamp() const;
    Allocator& allocator() const;

    void onRenderFrame(const Color& backgroundColor, RenderView& renderView) const;

    void jobDone();

    void addRequest(sp<RenderCommand> renderCommand) const;
    void addBackgroundRequest(const RenderLayer& layer, const V3& position);

public:
    struct Stub {
        Stub(uint64_t timestamp, const sp<Allocator::Pool>& allocatorPool, const sp<Executor>& executor, const sp<OCSQueue<RenderRequest>>& renderRequests);

        void onJobDone(const sp<Stub>& self);

        uint64_t _timestamp;
        Allocator _allocator;
        sp<Executor> _executor;
        sp<OCSQueue<RenderRequest>> _render_requests;

        sp<RenderCommandPipeline> _render_command_pipe_line;
        std::atomic<int32_t> _background_renderer_count;
    };

private:
    RenderRequest(sp<Stub> stub);

private:
    sp<Stub> _stub;

    friend struct Stub;
};

}

#endif
