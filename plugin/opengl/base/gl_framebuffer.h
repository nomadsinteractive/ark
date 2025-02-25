#pragma once

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/render_target.h"
#include "renderer/inf/resource.h"

namespace ark::plugin::opengl {

class GLFramebuffer final : public Resource {
public:
    GLFramebuffer(sp<Recycler> recycler, RenderTarget::Configure configure);
    ~GLFramebuffer() override;

    uint64_t id() override;

    void upload(GraphicsContext& graphicsContext) override;
    ResourceRecycleFunc recycle() override;

    const RenderTarget::Configure& configure() const;

private:
    sp<Recycler> _recycler;
    RenderTarget::Configure _configure;

    uint32_t _id;
};

}
