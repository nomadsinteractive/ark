#ifndef ARK_GRAPHICS_BASE_RENDER_LAYER_H_
#define ARK_GRAPHICS_BASE_RENDER_LAYER_H_

#include "core/collection/expirable_item_list.h"
#include "core/collection/list.h"
#include "core/inf/builder.h"
#include "core/types/class.h"

#include "graphics/inf/renderer.h"
#include "graphics/forwarding.h"

namespace ark {

//[[core::class]]
class ARK_API RenderLayer : public Renderer {
public:
// [[script::bindings::auto]]
    RenderLayer(const sp<Layer>& layer);

//  [[script::bindings::meta(absorb())]]
//  [[script::bindings::meta(expire())]]
//  [[script::bindings::meta(isExpired())]]

    virtual sp<RenderCommand> render(RenderRequest& renderRequest, float x, float y) override;

// [[script::bindings::auto]]
    void addRenderObject(const sp<RenderObject>& renderObject, const sp<Boolean>& expirable = sp<Boolean>::null());
// [[script::bindings::auto]]
    void removeRenderObject(const sp<RenderObject>& renderObject);

// [[script::bindings::auto]]
    void clear();

//  [[plugin::builder]]
    class BUILDER_IMPL1 : public Builder<RenderLayer> {
    public:
        BUILDER_IMPL1(BeanFactory& parent, const document& doc);

        virtual sp<RenderLayer> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Layer>> _layer;
        List<sp<Builder<RenderObject>>> _render_objects;
    };

//  [[plugin::builder("render-layer")]]
    class BUILDER_IMPL2 : public Builder<Renderer> {
    public:
        BUILDER_IMPL2(BeanFactory& parent, const document& doc);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        BUILDER_IMPL1 _builder_impl;
    };

private:
    sp<Layer> _layer;
    ExpirableItemList<RenderObject> _render_objects;

};

}

#endif
