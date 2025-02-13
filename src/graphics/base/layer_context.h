#pragma once

#include <deque>
#include <vector>

#include "core/base/api.h"
#include "core/base/timestamp.h"

#include "graphics/forwarding.h"
#include "graphics/base/render_layer.h"
#include "graphics/inf/renderable.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API LayerContext final : public Updatable {
public:
    struct ElementState {
        Optional<element_index_t> _index;
    };

public:
    LayerContext(sp<Shader> shader = nullptr, sp<ModelLoader> modelLoader = nullptr, sp<Vec3> position = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> discarded = nullptr, sp<Varyings> varyings = nullptr);

    bool update(uint64_t timestamp) override;

    const sp<Shader>& shader() const;

    const SafeVar<Vec3>& position() const;
    void setPosition(sp<Vec3> position);

    SafeVar<Boolean>& visible();
    const SafeVar<Boolean>& visible() const;

    const SafeVar<Boolean>& discarded() const;

    const sp<ModelLoader>& modelLoader() const;
    void setModelLoader(sp<ModelLoader> modelLoader);

    void add(sp<Renderable> renderable, sp<Updatable> updatable = nullptr, sp<Boolean> discarded = nullptr);
    void clear();
    void dispose();
    const sp<Varyings>& varyings() const;
    void setVaryings(sp<Varyings> varyings);

    void markDirty();

    bool processNewCreated();

    LayerContextSnapshot snapshot(RenderLayer renderLayer, const RenderRequest& renderRequest, const ShaderLayout& pipelineInput);

    bool ensureState(void* stateKey);
    ElementState& addElementState(void* key);

private:
    sp<Shader> _shader;
    sp<ModelLoader> _model_loader;

    SafeVar<Vec3> _position;
    SafeVar<Boolean> _visible;
    SafeVar<Boolean> _discarded;

    sp<Varyings> _varyings;

    bool _reload_requested;

    std::deque<std::pair<sp<Renderable>, Renderable::State>> _renderables;
    std::vector<sp<Renderable>> _renderable_created;

    std::unordered_map<void*, ElementState> _element_states;

    Timestamp _timestamp;

    friend class RenderLayerSnapshot;
};

}
