#ifndef ARK_GRAPHICS_BASE_RENDER_REQUEST_H_
#define ARK_GRAPHICS_BASE_RENDER_REQUEST_H_

#include <atomic>

#include "core/types/shared_ptr.h"
#include "core/base/object_pool.h"

#include "graphics/forwarding.h"
#include "graphics/base/render_command_pipeline.h"
#include "graphics/inf/render_command.h"
#include "graphics/inf/renderer.h"

namespace ark {

class RenderRequest {
public:
    RenderRequest(const sp<Executor>& executor, const sp<SurfaceController>& surfaceController, const sp<LockFreeStack<RenderRequest>>& renderRequestRecycler);
    RenderRequest(const RenderRequest& other) = default;
    RenderRequest(RenderRequest&& other) = default;
    RenderRequest() = default;

    RenderRequest& operator =(const RenderRequest& other) = default;
    RenderRequest& operator =(RenderRequest&& other) = default;

    void start(const sp<RenderCommandPipeline>& renderCommandPipeline);
    void finish();

    void addRequest(const sp<RenderCommand>& renderCommand);
    void addBackgroundRequest(const sp<Layer>& layer, float x, float y);

public:
    struct Stub {
        Stub(const sp<Executor>& executor, const sp<SurfaceController>& surfaceController, const sp<LockFreeStack<RenderRequest>>& renderRequestRecycler);

        void start(const sp<RenderCommandPipeline>& renderCommandPipeline);
        void onJobDone(const sp<Stub>& self);

        void sendRequest();

        sp<Executor> _executor;
        sp<RenderCommandPipeline> _render_command_pipe_line;
        sp<SurfaceController> _surface_controller;
        sp<LockFreeStack<RenderRequest>> _render_request_recycler;

        ObjectPool _render_command_pool;
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
