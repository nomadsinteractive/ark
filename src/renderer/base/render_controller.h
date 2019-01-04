#ifndef ARK_RENDERER_BASE_RENDER_CONTROLLER_H_
#define ARK_RENDERER_BASE_RENDER_CONTROLLER_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/collection/filtered_list.h"
#include "core/collection/list.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/resource_manager.h"
#include "renderer/inf/uploader.h"

namespace ark {

class ARK_API RenderController {
public:
    RenderController(const sp<RenderEngine>& renderEngine, const sp<ResourceManager>& resourceManager);

    const sp<RenderEngine>& renderEngine() const;
    const sp<ResourceManager>& resourceManager() const;

    sp<PipelineFactory> createPipelineFactory() const;
    sp<Dictionary<sp<Texture>>> createTextureBundle() const;
    sp<Texture> createTexture(uint32_t width, uint32_t height, const sp<Variable<bitmap>>& bitmapVariable, ResourceManager::UploadStrategy us = ResourceManager::US_ONCE_AND_ON_SURFACE_READY);

    Buffer makeBuffer(Buffer::Type type, Buffer::Usage usage, const sp<Uploader>& uploader) const;
    Buffer makeVertexBuffer(Buffer::Usage usage = Buffer::USAGE_DYNAMIC, const sp<Uploader>& uploader = nullptr) const;
    Buffer makeIndexBuffer(Buffer::Usage usage = Buffer::USAGE_DYNAMIC, const sp<Uploader>& uploader = nullptr) const;

    sp<Variable<uint64_t>> ticker() const;

    void addPreUpdateRequest(const sp<Runnable>& task, const sp<Boolean>& expired);

    void preUpdate();
    void deferUnref(const Box& box);

private:
    class Ticker : public Variable<uint64_t> {
    public:
        Ticker();

        virtual uint64_t val() override;

        void update();

    private:
        sp<Variable<uint64_t>> _ticker;
        uint64_t _tick;
    };


private:
    sp<RenderEngine> _render_engine;
    sp<ResourceManager> _resource_manager;
    sp<Ticker> _ticker;

    ListWithLifecycle<Runnable> _on_pre_update_request;
    List<Box> _defered_instances;
};

}

#endif
