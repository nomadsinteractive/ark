#pragma once

#include <vector>

#include "core/inf/builder.h"
#include "core/inf/holder.h"
#include "core/traits/visibility.h"
#include "core/types/shared_ptr.h"
#include "core/types/safe_ptr.h"
#include "core/types/safe_var.h"

#include "graphics/inf/renderer.h"
#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

//[[script::bindings::holder]]
//[[script::bindings::extends(Renderer)]]
class ARK_API Layer : public Renderer, public Holder {
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

    virtual void render(RenderRequest& renderRequest, const V3& position) override;
    virtual void traverse(const Visitor& visitor) override;

// [[script::bindings::auto]]
    void dispose();

//  [[script::bindings::property]]
    const SafeVar<Boolean>& visible() const;
//  [[script::bindings::property]]
    void setVisible(sp<Boolean> visible);

//  [[script::bindings::property]]
    const sp<ModelLoader>& modelLoader() const;

// [[script::bindings::property]]
    const sp<LayerContext>& context() const;

// [[script::bindings::auto]]
    void addRenderObject(const sp<RenderObject>& renderObject, const sp<Boolean>& disposed = nullptr);
// [[script::bindings::auto]]
    void clear();

//  [[plugin::builder]]
    class BUILDER : public Builder<Layer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Layer> build(const Scope& args) override;

    private:
        Layer::Type _type;
        SafePtr<Builder<RenderLayer>> _render_layer;
        SafePtr<Builder<ModelLoader>> _model_loader;
        SafePtr<Builder<Boolean>> _visible;
        SafePtr<Builder<Vec3>> _position;
        std::vector<sp<Builder<RenderObject>>> _render_objects;
    };

//  [[plugin::builder("layer")]]
    class BUILDER_IMPL2 : public Builder<Renderer> {
    public:
        BUILDER_IMPL2(BeanFactory& factory, const document& manifest);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        BUILDER _builder_impl;
    };

private:
    sp<LayerContext> _layer_context;

    friend class RenderLayer;

};

}
