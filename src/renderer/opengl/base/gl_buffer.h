#ifndef ARK_RENDERER_OPENGL_BASE_GL_BUFFER_H_
#define ARK_RENDERER_OPENGL_BASE_GL_BUFFER_H_

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/inf/resource.h"

#include "platform/gl/gl.h"

namespace ark {
namespace opengl {

class GLBuffer : public Buffer::Delegate {
public:
    GLBuffer(Buffer::Type type, Buffer::Usage usage, const sp<Recycler>& recycler);
    ~GLBuffer() override;

    virtual uint64_t id() override;
    virtual void upload(GraphicsContext& graphicsContext, const sp<Uploader>& uploader) override;
    virtual RecycleFunc recycle() override;

private:
    void doUpload(GraphicsContext& graphicsContext, Uploader& uploader);

private:
    GLenum _type;
    GLenum _usage;

    sp<Recycler> _recycler;

    uint32_t _id;
};


}
}

#endif
