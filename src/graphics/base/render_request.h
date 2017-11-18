#ifndef ARK_GRAPHICS_BASE_RENDER_REQUEST_H_
#define ARK_GRAPHICS_BASE_RENDER_REQUEST_H_

#include <atomic>

#include "core/types/shared_ptr.h"
#include "core/base/object_pool.h"

#include "graphics/forwarding.h"
#include "graphics/inf/render_command.h"

namespace ark {

class RenderRequest {
public:
    RenderRequest(const sp<Executor>& executor, const sp<SurfaceController>& surfaceController);

    void start(const sp<RenderCommandPipeline>& renderCommandPipeline);
    void finish();

    bool isFinished();

    void addRequest(const sp<RenderCommand>& renderCommand);
    void addBackgroundRequest(const sp<Layer>& layer, float x, float y);

public:
    struct Stub {
        Stub(const sp<SurfaceController>& surfaceController);

        void start(const sp<RenderCommandPipeline>& renderCommandPipeline);
        void onJobDone();

        void sendRequest();

        sp<RenderCommandPipeline> _render_command_pipe_line;
        sp<SurfaceController> _surface_controller;

        std::atomic<int32_t> _background_renderer_count;
    };

private:
    sp<Executor> _executor;
    ObjectPool _render_command_pool;

    sp<Stub> _stub;
};

}

#endif
