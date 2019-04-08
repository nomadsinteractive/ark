#ifndef ARK_GRAPHICS_BASE_LAYER_H_
#define ARK_GRAPHICS_BASE_LAYER_H_

#include <vector>

#include "core/collection/filtered_list.h"
#include "core/collection/list.h"
#include "core/inf/builder.h"
#include "core/types/class.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_layer.h"
#include "graphics/inf/renderer.h"
#include "graphics/forwarding.h"

namespace ark {

class ARK_API Layer : public Renderer {
public:
// [[script::bindings::auto]]
    Layer(const sp<RenderLayer>& renderer);

//  [[script::bindings::meta(expire())]]
//  [[script::bindings::meta(isExpired())]]

    virtual void render(RenderRequest& renderRequest, float x, float y) override;

    void draw(float x, float y, const sp<RenderObject>& renderObject);

// [[script::bindings::property]]
    const sp<RenderLayer>& renderer() const;
// [[script::bindings::property]]
    const sp<LayerContext>& context() const;

// [[script::bindings::auto]]
    void addRenderObject(const sp<RenderObject>& renderObject, const sp<Boolean>& disposed = sp<Boolean>::null());
// [[script::bindings::auto]]
    void removeRenderObject(const sp<RenderObject>& renderObject);
// [[script::bindings::auto]]
    void clear();

//  [[plugin::builder]]
    class BUILDER_IMPL1 : public Builder<Layer> {
    public:
        BUILDER_IMPL1(BeanFactory& factory, const document& manifest);

        virtual sp<Layer> build(const sp<Scope>& args) override;

    private:
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
    Layer(const sp<RenderLayer::Stub>& stub);

private:
    sp<RenderLayer> _render_layer;
    sp<LayerContext> _layer_context;

    friend class RenderLayer;
};

}

#endif
