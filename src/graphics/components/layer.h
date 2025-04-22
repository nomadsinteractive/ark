#pragma once

#include "core/inf/builder.h"
#include "core/inf/debris.h"
#include "core/inf/wirable.h"
#include "core/types/shared_ptr.h"
#include "core/impl/builder/safe_builder.h"
#include "core/types/safe_var.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Layer final {
public:
// [[script::bindings::auto]]
    Layer();
    Layer(sp<LayerContext> layerContext);

// [[script::bindings::auto]]
    void discard();

//  [[script::bindings::property]]
    const sp<Shader>& shader() const;

//  [[script::bindings::property]]
    const SafeVar<Vec3>& position() const;
//  [[script::bindings::property]]
    void setPosition(sp<Vec3> position);

//  [[script::bindings::property]]
    const SafeVar<Boolean>& visible() const;
//  [[script::bindings::property]]
    void setVisible(sp<Boolean> visible);

//  [[script::bindings::property]]
    const SafeVar<Boolean>& discarded() const;

//  [[script::bindings::property]]
    sp<ModelLoader> modelLoader() const;

    void pushFront(sp<Renderable> renderable, sp<Boolean> discarded = nullptr);
    void pushBack(sp<Renderable> renderable, sp<Boolean> discarded = nullptr);
//  [[script::bindings::auto]]
    void pushFront(const sp<RenderObject>& renderObject, const sp<Boolean>& discarded = nullptr);
//  [[script::bindings::auto]]
    void pushBack(const sp<RenderObject>& renderObject, const sp<Boolean>& discarded = nullptr);
//  [[script::bindings::auto]]
    void clear();

    const sp<LayerContext>& context() const;

//  [[plugin::builder]]
    class BUILDER final : public Builder<Layer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Layer> build(const Scope& args) override;

    private:
        SafeBuilder<RenderLayer> _render_layer;
        SafeBuilder<ModelLoader> _model_loader;
        SafeBuilder<Boolean> _visible;
        SafeBuilder<Vec3> _position;
    };

private:
    sp<LayerContext> _layer_context;

    friend class RenderLayer;
};

}
