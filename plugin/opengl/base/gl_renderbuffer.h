#pragma once

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/resource.h"

namespace ark::plugin::opengl {

class GLRenderbuffer final : public Resource {
public:
    GLRenderbuffer(const sp<Recycler>& recycler);
    ~GLRenderbuffer() override;

    uint64_t id() override;

    void upload(GraphicsContext& graphicsContext) override;
    ResourceRecycleFunc recycle() override;

private:
    sp<Recycler> _recycler;
    uint32_t _id;
};

}
