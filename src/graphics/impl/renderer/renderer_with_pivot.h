#ifndef ARK_GRAPHICS_IMPL_RENDERER_RENDERER_WITH_PIVOT_H_
#define ARK_GRAPHICS_IMPL_RENDERER_RENDERER_WITH_PIVOT_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

namespace ark {

class RendererWithPivot : public Renderer {
public:
    enum Pivot {
        NONE = 0,
        LEFT = 1,
        RIGHT = 2,
        CENTER_HORIZONTAL = 3,
        TOP = 4,
        BOTTOM = 8,
        CENTER_VERTICAL = 12,
        CENTER = CENTER_VERTICAL | CENTER_HORIZONTAL
    };

    RendererWithPivot(const sp<Renderer>& renderer, Pivot pivot);

    virtual void render(RenderCommandPipeline& pipeline, float x, float y) override;

//  [[plugin::style("pivot")]]
    class STYLE : public Builder<Renderer> {
    public:
        STYLE(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& value);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        Pivot _pivot;
    };

private:
    sp<Renderer> _renderer;
    sp<Block> _block;
    Pivot _pivot;

};

}

#endif
