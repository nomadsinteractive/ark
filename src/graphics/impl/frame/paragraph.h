#ifndef ARK_GRAPHICS_IMPL_FRAME_PARAGRAPH_H_
#define ARK_GRAPHICS_IMPL_FRAME_PARAGRAPH_H_

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
class Paragraph : public Renderer, public Block {
public:
    Paragraph(const sp<Text>& characters);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

    virtual const sp<Size>& size() override;

//  [[plugin::builder("paragraph")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Text>> _characters;
        sp<Builder<LayoutParam>> _layout_param;
        SafePtr<Builder<String>> _string;
    };

private:
    sp<RenderLayer> _render_layer;
    sp<Text> _characters;
};

}

#endif
