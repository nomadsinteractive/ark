#ifndef ARK_GRAPHICS_IMPL_FRAME_BAR_H_
#define ARK_GRAPHICS_IMPL_FRAME_BAR_H_

#include "core/inf/builder.h"
#include "core/collection/list.h"
#include "core/types/class.h"

#include "graphics/inf/renderer.h"
#include "graphics/inf/block.h"

namespace ark {

//[[core::class]]
class Bar : public Renderer, public Block {
public:
    Bar(const sp<Layer>& layer, const sp<RenderObject>& bottom, const sp<RenderObject>& bolierplate, const sp<RenderObject>& top, const sp<VV>& direction, const sp<Size>& size);

    virtual void render(RenderRequest& renderRequest, float x, float y) override;

    virtual const sp<Size>& size() override;

//  [[plugin::builder("bar")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Layer>> _layer;
        sp<Builder<Size>> _size;
        sp<Builder<RenderObject>> _bottom, _top, _boilerplate;
        sp<Builder<VV>> _direction;
    };

private:
    void update();

private:
    sp<RenderObject> _bottom;
    sp<RenderObject> _boilerplate;
    sp<RenderObject> _top;
    sp<VV> _direction;
    sp<Size> _size;

    sp<RenderLayer> _render_layer;
    float _bar_width;
    float _bar_height;
};

}

#endif
