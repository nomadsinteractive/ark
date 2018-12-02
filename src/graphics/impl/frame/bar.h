#ifndef ARK_GRAPHICS_IMPL_FRAME_BAR_H_
#define ARK_GRAPHICS_IMPL_FRAME_BAR_H_

#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/renderer.h"
#include "graphics/inf/block.h"

namespace ark {

//[[core::class]]
class Bar : public Renderer, public Block {
public:
    Bar(const sp<Layer>& layer, const sp<RenderObject>& bottom, const sp<RenderObject>& bolierplate, const sp<RenderObject>& top, const sp<Vec>& direction, const sp<Size>& size);

    virtual void render(RenderRequest& renderRequest, float x, float y) override;

    virtual const SafePtr<Size>& size() override;

//  [[plugin::builder("bar")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Layer>> _layer;
        sp<Builder<Size>> _size;
        SafePtr<Builder<RenderObject>> _bottom, _top;
        sp<Builder<RenderObject>> _boilerplate;
        sp<Builder<Vec>> _direction;
    };

private:
    void update();

private:
    sp<RenderObject> _bottom;
    sp<RenderObject> _boilerplate;
    sp<RenderObject> _top;
    sp<Vec> _direction;
    SafePtr<Size> _size;

    sp<RenderLayer> _render_layer;
    float _bar_width;
    float _bar_height;
};

}

#endif
