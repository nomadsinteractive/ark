#ifndef ARK_GRAPHICS_IMPL_FRAME_LABEL_H_
#define ARK_GRAPHICS_IMPL_FRAME_LABEL_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/block.h"
#include "graphics/inf/renderer.h"
#include "graphics/util/alphabets.h"

#include "renderer/forwarding.h"

namespace ark {

//[[core::class]]
class Label : public Renderer, public Block {
public:
    Label(const sp<Characters>& characters);

    virtual void render(RenderRequest& renderRequest, float x, float y) override;

    virtual const sp<Size>& size() override;

//  [[plugin::builder("label")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Characters>> _characters;
    };

private:
    sp<Characters> _characters;

};

}

#endif
