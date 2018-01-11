#include "renderer/base/render_controller.h"

#include "core/inf/runnable.h"
#include "core/util/log.h"

namespace ark {

void RenderController::addPreUpdateRequest(const sp<Runnable>& task, const sp<Boolean>& expired)
{
    if(expired)
        _on_pre_update_request.push_back(task, expired);
    else
    {
        const sp<Expired> e = task.as<Expired>();
        DCHECK(e, "Adding an unexpired running task, it's that what you REALLY want?");
        _on_pre_update_request.push_back(task, e);
    }
}

void RenderController::preUpdate()
{
#ifdef ARK_FLAG_DEBUG
    static int i = 0;
    if(i ++ == 600)
    {
        LOGD("_on_pre_update_request: %d", _on_pre_update_request.size());
        i = 0;
    }
#endif
    _defered_instances.clear();
    for(const sp<Runnable>& runnable : _on_pre_update_request)
        runnable->run();
}

void RenderController::deferUnref(const Box& box)
{
    _defered_instances.push_back(box);
}

}
