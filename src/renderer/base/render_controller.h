#ifndef ARK_RENDERER_BASE_RENDER_CONTROLLER_H_
#define ARK_RENDERER_BASE_RENDER_CONTROLLER_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/collection/filtered_list.h"
#include "core/collection/list.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API RenderController {
public:
    RenderController(const sp<RenderEngine>& renderEngine);

    const sp<RenderEngine>& renderEngine() const;

    void addPreUpdateRequest(const sp<Runnable>& task, const sp<Boolean>& expired);

    void preUpdate();
    void deferUnref(const Box& box);

private:
    sp<RenderEngine> _render_engine;

    ListWithLifecycle<Runnable> _on_pre_update_request;
    List<Box> _defered_instances;
};

}

#endif
