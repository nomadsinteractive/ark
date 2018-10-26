#ifndef ARK_RENDERER_BASE_RENDER_CONTROLLER_H_
#define ARK_RENDERER_BASE_RENDER_CONTROLLER_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/collection/list_with_lifecycle.h"
#include "core/collection/list.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API RenderController {
public:

    void addPreUpdateRequest(const sp<Runnable>& task, const sp<Boolean>& expired);

    void preUpdate();
    void deferUnref(const Box& box);

private:
    ListWithLifecycle<Runnable> _on_pre_update_request;

    List<Box> _defered_instances;
};

}

#endif
