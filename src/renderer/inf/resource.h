#ifndef ARK_RENDERER_INF_RESOURCE_H_
#define ARK_RENDERER_INF_RESOURCE_H_

#include <functional>

#include "core/base/api.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Resource {
public:
    typedef std::function<void(GraphicsContext&)> RecycleFunc;

    virtual ~Resource() = default;

    virtual uint64_t id() = 0;

    virtual void upload(GraphicsContext& graphicsContext, const sp<Uploader>& uploader) = 0;
    virtual RecycleFunc recycle() = 0;
};

}

#endif
