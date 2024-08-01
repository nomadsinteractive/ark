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
        MODEL_TRAIT_FIXED_VERTEX_COUNT = 1
    };

    ModelLoader(Enum::RenderMode renderMode);
    virtual ~ModelLoader() = default;

//[[script::bindings::property]]
    Enum::RenderMode renderMode() const;
    [[nodiscard]]
//[[script::bindings::auto]]
    virtual sp<Model> loadModel(int32_t type) = 0;
    [[nodiscard]]
    virtual sp<RenderCommandComposer> makeRenderCommandComposer() = 0;
    virtual void initialize(PipelineBindings& pipelineBindings) = 0;

    class ARK_API Importer {
    public:
        virtual ~Importer() = default;

        [[nodiscard]]
        virtual Model import(const Manifest& manifest, MaterialBundle& materialBundle) = 0;
    };

private:
    Enum::RenderMode _render_mode;
};

}
