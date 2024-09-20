#pragma once

#include <vector>

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/render_target.h"
#include "renderer/inf/resource.h"

namespace ark::opengl {

class GLFramebuffer final : public Resource {
public:
    GLFramebuffer(sp<Recycler> recycler, RenderTarget::CreateConfigure configure);
    ~GLFramebuffer() override;

    uint64_t id() override;

    void upload(GraphicsContext& graphicsContext) override;
    ResourceRecycleFunc recycle() override;

private:
    sp<Recycler> _recycler;
    RenderTarget::CreateConfigure _configure;

    uint32_t _id;
};

}
