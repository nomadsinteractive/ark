#pragma once

#include <vector>

#include "core/inf/builder.h"
#include "core/inf/debris.h"
#include "core/types/shared_ptr.h"
#include "core/types/safe_builder.h"
#include "core/types/safe_var.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Layer final {
public:
//  [[script::bindings::enumeration]]
    enum Type {
        TYPE_UNSPECIFIED,
        TYPE_DYNAMIC,
        TYPE_STATIC,
        TYPE_TRANSIENT
    };

public:
// [[script::bindings::auto]]
    Layer(sp<LayerContext> layerContext = nullptr);

// [[script::bindings::auto]]
    void dispose();

//  [[script::bindings::property]]
    const SafeVar<Boolean>& visible() const;
//  [[script::bindings::property]]
    void setVisible(sp<Boolean> visible);

//  [[script::bindings::property]]
    sp<ModelLoader> modelLoader() const;

// [[script::bindings::property]]
    const sp<LayerContext>& context() const;

// [[script::bindings::auto]]
    void addRenderObject(const sp<RenderObject>& renderObject, const sp<Boolean>& discarded = nullptr);
// [[script::bindings::auto]]
    void clear();

//  [[plugin::builder]]
    class BUILDER : public Builder<Layer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Layer> build(const Scope& args) override;

    private:
        SafeBuilder<RenderLayer> _render_layer;
        SafeBuilder<ModelLoader> _model_loader;
        SafeBuilder<Boolean> _visible;
        SafeBuilder<Vec3> _position;
        std::vector<sp<Builder<RenderObject>>> _render_objects;
    };

private:
    sp<LayerContext> _layer_context;

    friend class RenderLayer;
};

}
