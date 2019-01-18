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
    RenderRequest() = default;
    RenderRequest(const sp<Executor>& executor, const sp<SurfaceController>& surfaceController, LFStack<RenderRequest>& renderRequestRecycler);
    DEFAULT_COPY_AND_ASSIGN(RenderRequest);

    void start(const sp<RenderCommandPipeline>& renderCommandPipeline);
    void finish();

    void addRequest(const sp<RenderCommand>& renderCommand);
    void addBackgroundRequest(const Layer& layer, float x, float y);

public:
    struct Stub {
        Stub(const sp<Executor>& executor, const sp<SurfaceController>& surfaceController, LFStack<RenderRequest>& renderRequestRecycler);

        void start(const sp<RenderCommandPipeline>& renderCommandPipeline);
        void onJobDone(const sp<Stub>& self);

        void sendRequest();

        sp<Executor> _executor;
        sp<RenderCommandPipeline> _render_command_pipe_line;
        sp<SurfaceController> _surface_controller;

        LFStack<RenderRequest>& _render_request_recycler;

        ObjectPool _object_pool;
        std::atomic<int32_t> _background_renderer_count;
    };

private:
    RenderRequest(const sp<Stub>& stub);

private:
    sp<Stub> _stub;
    sp<Runnable> _callback;

    friend struct Stub;
};

}

#endif
