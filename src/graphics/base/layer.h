#ifndef ARK_GRAPHICS_BASE_LAYER_H_
#define ARK_GRAPHICS_BASE_LAYER_H_

#include <vector>

#include "core/inf/builder.h"
#include "core/inf/holder.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/renderer.h"
#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::holder]]
class ARK_API Layer : public Renderer, public Holder {
public:
//  [[script::bindings::enumeration]]
    enum Type {
        TYPE_UNSPECIFIED,
        TYPE_DYNAMIC,
        TYPE_STATIC
    };

public:
// [[script::bindings::auto]]
    Layer(const sp<RenderLayer>& renderLayer = nullptr, Layer::Type type = Layer::TYPE_DYNAMIC);

    virtual void render(RenderRequest& renderRequest, float x, float y) override;
    virtual void traverse(const Visitor& visitor) override;

    void draw(float x, float y, const sp<RenderObject>& renderObject);

// [[script::bindings::auto]]
    void attach(const sp<RenderLayer>& renderLayer);
// [[script::bindings::auto]]
    void detach();

// [[script::bindings::property]]
    const sp<RenderLayer>& renderer() const;
// [[script::bindings::property]]
    const sp<LayerContext>& context() const;

// [[script::bindings::auto]]
    void addRenderObject(const sp<RenderObject>& renderObject, const sp<Boolean>& disposed = sp<Boolean>::null());
// [[script::bindings::auto]]
    void clear();

//  [[plugin::builder]]
    class BUILDER_IMPL1 : public Builder<Layer> {
    public:
        BUILDER_IMPL1(BeanFactory& factory, const document& manifest);

        virtual sp<Layer> build(const sp<Scope>& args) override;

    private:
        Layer::Type _type;
        sp<Builder<RenderLayer>> _render_layer;
        std::vector<sp<Builder<RenderObject>>> _render_objects;
    };

//  [[plugin::builder("layer")]]
    class BUILDER_IMPL2 : public Builder<Renderer> {
    public:
        BUILDER_IMPL2(BeanFactory& factory, const document& manifest);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        BUILDER_IMPL1 _builder_impl;
    };

private:
    sp<RenderLayer> _render_layer;
    sp<LayerContext> _layer_context;

    friend class RenderLayer;

};

}

#endif
