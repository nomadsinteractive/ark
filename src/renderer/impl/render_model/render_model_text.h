#ifndef ARK_RENDERER_IMPL_RENDER_MODEL_RENDER_MODEL_TEXT_H_
#define ARK_RENDERER_IMPL_RENDER_MODEL_RENDER_MODEL_TEXT_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/render_model.h"
#include "renderer/inf/resource.h"

namespace ark {

class GLModelText : public RenderModel {
private:
    class Stub : public Variable<sp<Resource>> {
    public:
        Stub(const sp<RenderController>& renderController, const sp<Alphabet>& alphabet, uint32_t textureWidth, uint32_t textureHeight);

        void reset(uint32_t textureWidth, uint32_t textureHeight);

        bool checkUnpreparedCharacter(const Layer::Snapshot& renderContext);
        bool prepare(const Layer::Snapshot& snapshot, bool allowReset);

        virtual sp<Resource> val() override;

    private:
        bool prepareOne(int32_t c);
        void clear();

    private:
        sp<RenderController> _render_controller;
        sp<Alphabet> _alphabet;
        bitmap _font_glyph;
        sp<Texture> _texture;
        sp<Atlas> _atlas;
        sp<RenderModel> _delegate;
        sp<Size> _size;

        uint32_t _flowx, _flowy;
        int32_t _max_glyph_height;

        std::unordered_set<int32_t> _characters;

        friend class GLModelText;
    };

public:
    GLModelText(const sp<RenderController>& renderController, const sp<Alphabet>& alphabet, uint32_t textureWidth, uint32_t textureHeight);

    virtual sp<ShaderBindings> makeShaderBindings(const RenderController& renderController, const sp<PipelineLayout>& pipelineLayout) override;

    virtual void postSnapshot(const Layer::Snapshot& snapshot) override;

    virtual void start(ModelBuffer& buf, RenderController& renderController, const Layer::Snapshot& snapshot) override;
    virtual void load(ModelBuffer& buf, int32_t type, const V& scale) override;
    virtual Metrics measure(int32_t type) override;

//  [[plugin::resource-loader("text")]]
    class BUILDER : public Builder<RenderModel> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<RenderModel> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        sp<Builder<Alphabet>> _alphabet;
        uint32_t _texture_width, _texture_height;
    };

private:
    sp<Stub> _stub;
};

}

#endif
