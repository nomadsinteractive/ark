#include "renderer/base/gl_recycler.h"

#include "renderer/inf/render_resource.h"

namespace ark {

namespace {

class GLRecyclerFunction : public RenderResource {
public:
    GLRecyclerFunction(uint32_t id, std::function<void(uint32_t)> recycler)
        : _id(id), _recycler(std::move(recycler)) {
    }

    virtual uint32_t id() override {
        return _id;
    }

    virtual void prepare(GraphicsContext& /*graphicsContext*/) override {
    }

    virtual void recycle(GraphicsContext& /*graphicsContext*/) override {
        _recycler(_id);
    }

private:
    uint32_t _id;
    std::function<void(uint32_t)> _recycler;
};

}

void GLRecycler::recycle(const sp<RenderResource>& resource)
{
    _items.push(resource);
}

void GLRecycler::recycle(uint32_t id, std::function<void (uint32_t)> recycler)
{
    _items.push(sp<GLRecyclerFunction>::make(id, std::move(recycler)));
}

void GLRecycler::doRecycling(GraphicsContext& graphicsContext)
{
    for(const sp<RenderResource>& resource : _items.clear())
        resource->recycle(graphicsContext);
}

}
