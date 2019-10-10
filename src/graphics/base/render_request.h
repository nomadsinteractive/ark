#ifndef ARK_GRAPHICS_BASE_RENDER_REQUEST_H_
#define ARK_GRAPHICS_BASE_RENDER_REQUEST_H_

#include <atomic>

#include "core/base/api.h"
#include "core/concurrent/one_consumer_synchronized.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API RenderRequest {
public:
    RenderRequest() = default;
    RenderRequest(const sp<Executor>& executor, const sp<OCSQueue<sp<RenderCommand>>>& renderCommands);
    DEFAULT_COPY_AND_ASSIGN(RenderRequest);

    void finish();

    void addRequest(const sp<RenderCommand>& renderCommand);
    void addBackgroundRequest(const RenderLayer& layer, const V3& position);

public:
    struct Stub {
        Stub(const sp<Executor>& executor, const sp<OCSQueue<sp<RenderCommand>>>& renderCommands);

        void onJobDone();

        sp<Executor> _executor;
        sp<OCSQueue<sp<RenderCommand>>> _render_commands;

        sp<RenderCommandPipeline> _render_command_pipe_line;
        std::atomic<int32_t> _background_renderer_count;
    };

private:
    RenderRequest(const sp<Stub>& stub);

private:
    sp<Stub> _stub;

    friend struct Stub;
};

}

#endif
