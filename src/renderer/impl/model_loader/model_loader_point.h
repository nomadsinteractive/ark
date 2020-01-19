#ifndef ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_POINT_H_
#define ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_POINT_H_

#include "core/types/shared_ptr.h"

#include "renderer/base/model.h"
#include "renderer/inf/model_loader.h"

namespace ark {

class ModelLoaderPoint : public ModelLoader {
public:
    ModelLoaderPoint(const sp<Atlas>& atlas);

    virtual sp<RenderCommandComposer> makeRenderCommandComposer() override;

    virtual void initialize(ShaderBindings& shaderBindings) override;
    virtual void postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot) override;

    virtual Model load(int32_t type) override;

    class BUILDER : public Builder<ModelLoader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<ModelLoader> build(const Scope& args) override;

    private:
        sp<Builder<Atlas>> _atlas;

    };

private:
    sp<Atlas> _atlas;

};

}

#endif
