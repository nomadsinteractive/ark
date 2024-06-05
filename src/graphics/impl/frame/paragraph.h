#pragma once

#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"


namespace ark {

//[[core::class]]
class Paragraph : public Renderer {
public:
[[deprecated]]
    Paragraph(const sp<Text>& characters);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

    const sp<Size>& size();

//  [[plugin::builder("paragraph")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Text>> _characters;
        SafePtr<Builder<String>> _string;
    };

private:
    sp<RenderLayer> _render_layer;
    sp<Text> _characters;
};

}
