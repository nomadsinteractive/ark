#include "renderer/base/render_controller.h"

#include "core/inf/runnable.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"

#include "renderer/base/recycler.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/resource_manager.h"
#include "renderer/base/texture.h"
#include "renderer/inf/renderer_factory.h"

#include "platform/platform.h"

namespace ark {

namespace {

class GLTextureBundle : public Dictionary<sp<Texture>> {
public:
    GLTextureBundle(const sp<RendererFactory>& rendererFactory, const sp<Recycler>& recycler, const sp<Dictionary<bitmap>>& bitmapLoader, const sp<Dictionary<bitmap>>& bitmapBoundsLoader)
        : _renderer_factory(rendererFactory), _recycler(recycler), _bitmap_loader(bitmapLoader), _bitmap_bounds_loader(bitmapBoundsLoader)
    {
    }

    virtual sp<Texture> get(const String& name) override {
        const bitmap bitmapBounds = _bitmap_bounds_loader->get(name);
        DCHECK(bitmapBounds, "Texture resource \"%s\" not found", name.c_str());
        return _renderer_factory->createTexture(_recycler, bitmapBounds->width(), bitmapBounds->height(), sp<Variable<bitmap>::Get>::make(_bitmap_loader, name));
    }

private:
    sp<RendererFactory> _renderer_factory;
    sp<Recycler> _recycler;

    sp<Dictionary<bitmap>> _bitmap_loader;
    sp<Dictionary<bitmap>> _bitmap_bounds_loader;
};

}

RenderController::Ticker::Ticker(const sp<Variable<uint64_t>>& ticker)
    : _ticker(ticker), _tick(0)
{
}

uint64_t RenderController::Ticker::val()
{
    DTHREAD_CHECK(THREAD_ID_CORE);
    return _tick;
}

void RenderController::Ticker::update()
{
    _tick = _ticker->val();
}

RenderController::RenderController(const sp<RenderEngine>& renderEngine, const sp<ResourceManager>& resourceManager, const sp<Variable<uint64_t>>& ticker)
    : _render_engine(renderEngine), _resource_manager(resourceManager), _ticker(sp<Ticker>::make(ticker))
{
    _named_buffers[NamedBuffer::NAME_QUADS] = NamedBuffer::Quads::make(*this);
    _named_buffers[NamedBuffer::NAME_NINE_PATCH] = NamedBuffer::NinePatch::make(*this);
}

const sp<RenderEngine>& RenderController::renderEngine() const
{
    return _render_engine;
}

const sp<ResourceManager>& RenderController::resourceManager() const
{
    return _resource_manager;
}

sp<PipelineFactory> RenderController::createPipelineFactory() const
{
    return _render_engine->rendererFactory()->createPipelineFactory();
}

sp<Dictionary<sp<Texture>>> RenderController::createTextureBundle() const
{
    return sp<GLTextureBundle>::make(_render_engine->rendererFactory(), _resource_manager->recycler(), _resource_manager->bitmapLoader(), _resource_manager->bitmapBoundsLoader());
}

sp<Texture> RenderController::createTexture(uint32_t width, uint32_t height, const sp<Variable<bitmap>>& bitmap, ResourceManager::UploadStrategy us)
{
    const sp<Texture> texture = _render_engine->rendererFactory()->createTexture(_resource_manager->recycler(), width, height, bitmap);
    _resource_manager->upload(texture, nullptr, us);
    return texture;
}

Buffer RenderController::makeVertexBuffer(Buffer::Usage usage, const sp<Uploader>& uploader) const
{
    return makeBuffer(Buffer::TYPE_VERTEX, usage, uploader);
}

Buffer RenderController::makeIndexBuffer(Buffer::Usage usage, const sp<Uploader>& uploader) const
{
    return makeBuffer(Buffer::TYPE_INDEX, usage, uploader);
}

Buffer RenderController::makeBuffer(Buffer::Type type, Buffer::Usage usage, const sp<Uploader>& uploader) const
{
    Buffer buffer(_render_engine->rendererFactory()->createBuffer(type, usage));
    _resource_manager->uploadBuffer(buffer, uploader, uploader ? ResourceManager::US_ONCE_AND_ON_SURFACE_READY : ResourceManager::US_ON_SURFACE_READY);
    return buffer;
}

sp<Variable<uint64_t>> RenderController::ticker() const
{
    return _ticker;
}

void RenderController::addPreUpdateRequest(const sp<Runnable>& task, const sp<Boolean>& expired)
{
    if(expired)
        _on_pre_update_request.push_back(task, expired);
    else
    {
        const sp<Lifecycle> e = task.as<Lifecycle>();
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
    _ticker->update();
    for(const sp<Runnable>& runnable : _on_pre_update_request)
        runnable->run();
}

void RenderController::deferUnref(const Box& box)
{
    _defered_instances.push_back(box);
}

const sp<NamedBuffer>& RenderController::getNamedBuffer(NamedBuffer::Name name) const
{
    return _named_buffers[name];
}

}
