#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/enum.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API ModelLoader {
public:
    enum ModelTrait {
        MODEL_TRAIT_NONE = 0,
        MODEL_TRAIT_FIXED_VERTEX_COUNT = 1,
        MODEL_TRAIT_DISALLOW_CACHE = 2,
    };

    ModelLoader(enums::DrawMode renderMode, sp<Texture> texture, ModelTrait trait = MODEL_TRAIT_NONE);
    virtual ~ModelLoader() = default;

    void bind(const PipelineBindings& pipelineBindings) const;

//  [[script::bindings::property]]
    enums::DrawMode renderMode() const;
//  [[script::bindings::property]]
    const sp<Texture>& texture() const;

    ModelTrait trait() const;

    [[nodiscard]]
//  [[script::bindings::auto]]
    virtual sp<Model> loadModel(int32_t resid) = 0;
    [[nodiscard]]
    virtual sp<DrawingContextComposer> makeRenderCommandComposer(const Shader& shader) = 0;

private:
    enums::DrawMode _render_mode;
    sp<Texture> _texture;
    ModelTrait _trait;
};

}
