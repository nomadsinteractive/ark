#ifndef ARK_GRAPHICS_IMPL_FRAME_TEXT_H_
#define ARK_GRAPHICS_IMPL_FRAME_TEXT_H_

#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/block.h"
#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"

#include "app/view/layout_param.h"

namespace ark {

//[[core::class]]
class Text : public Renderer, public Block {
public:
    Text(const sp<Characters>& characters);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

    virtual const sp<Size>& size() override;

//  [[plugin::builder("text")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Characters>> _characters;
        sp<Builder<LayoutParam>> _layout_param;
        SafePtr<Builder<String>> _text;
    };

private:
    sp<RenderLayer> _render_layer;
    sp<Characters> _characters;
};

}

#endif
