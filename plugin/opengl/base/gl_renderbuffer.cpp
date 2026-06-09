#include "opengl/base/gl_renderbuffer.h"

#include "core/util/log.h"

#include "renderer/base/recycler.h"
#include "renderer/inf/recyclable.h"

#include "platform/gl/gl.h"

namespace ark::plugin::opengl {

namespace {

class RecyclableGLRenderbuffer final : public Recyclable {
public:
    RecyclableGLRenderbuffer(const uint32_t id)
        : _id(id) {
    }

    ~RecyclableGLRenderbuffer() override {
        LOGD("Deleting GLRenderbuffer[%d]", _id);
        glDeleteRenderbuffers(1, &_id);
    }

private:
    uint32_t _id;
};

}

GLRenderbuffer::GLRenderbuffer(const sp<Recycler>& recycler)
    : _recycler(recycler), _id(0)
{
}

GLRenderbuffer::~GLRenderbuffer()
{
    if(id())
        _recycler->recycle(toRecyclable());
}

uint64_t GLRenderbuffer::id()
{
    return _id;
}

void GLRenderbuffer::upload(GraphicsContext& /*graphicsContext*/)
{
    if(_id == 0)
        glGenRenderbuffers(1, &_id);
}

op<Recyclable> GLRenderbuffer::toRecyclable()
{
    op<Recyclable> recyclable(new RecyclableGLRenderbuffer(_id));
    _id = 0;
    return recyclable;
}

}