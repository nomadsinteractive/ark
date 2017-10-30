#ifndef ARK_RENDERER_BASE_RENDER_CONTROLLER_H_
#define ARK_RENDERER_BASE_RENDER_CONTROLLER_H_

#include "core/forwarding.h"
#include "core/collection/expirable_item_list.h"
#include "core/collection/list.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class RenderController {
public:

    void addPreUpdateRequest(const sp<Runnable>& task);
    void addPreUpdateRequest(const sp<Runnable>& task, const sp<Boolean>& expired);

    void preUpdate();
    void deferUnref(const Box& box);

private:
    ExpirableItemList<Runnable> _on_pre_update_request;

    List<Box> _defered_instances;
};

}

#endif
