#pragma once

#include "core/base/api.h"
#include "core/base/enum.h"
#include "core/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/atlas.h"

namespace ark {

class ARK_API ModelLoader {
public:
    enum ModelTrait {
        MODEL_TRAIT_NONE = 0,
        MODEL_TRAIT_FIXED_VERTEX_COUNT = 1,
        MODEL_TRAIT_DISALLOW_CACHE = 2,
    };

    ModelLoader(Enum::DrawMode renderMode, sp<Texture> texture, ModelTrait trait = MODEL_TRAIT_NONE);
    virtual ~ModelLoader() = default;

    void bind(const PipelineBindings& pipelineBindings) const;

//  [[script::bindings::property]]
    Enum::DrawMode renderMode() const;
//  [[script::bindings::property]]
    const sp<Texture>& texture() const;

    ModelTrait trait() const;

    [[nodiscard]]
//  [[script::bindings::auto]]
    virtual sp<Model> loadModel(int32_t type) = 0;
    [[nodiscard]]
    virtual sp<DrawingContextComposer> makeRenderCommandComposer(const Shader& shader) = 0;

    class ARK_API Importer {
    public:
        virtual ~Importer() = default;

        [[nodiscard]]
        virtual Model import(const Manifest& manifest, MaterialBundle& materialBundle) = 0;
    };

private:
    Enum::DrawMode _render_mode;
    sp<Texture> _texture;
    ModelTrait _trait;
};

}
