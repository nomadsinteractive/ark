#ifndef ARK_RENDERER_OPENGL_BASE_GL_RENDERBUFFER_H_
#define ARK_RENDERER_OPENGL_BASE_GL_RENDERBUFFER_H_

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/resource.h"

namespace ark {
namespace opengl {

class GLRenderbuffer : public Resource {
public:
    GLRenderbuffer(const sp<Recycler>& recycler);
    ~GLRenderbuffer() override;

    virtual uint64_t id() override;

    virtual void upload(GraphicsContext& graphicsContext) override;
    virtual ResourceRecycleFunc recycle() override;

private:
    sp<Recycler> _recycler;

    uint32_t _id;
};

}
}

#endif
