#pragma once

#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"

namespace ark {

class Label : public Renderer {
public:
[[deprecated]]
    Label(const sp<Text>& characters);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

    const sp<Size>& size();

//  [[plugin::builder("label")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Text>> _characters;
        SafePtr<Builder<String>> _text;
    };

private:
    sp<Text> _characters;
};

}
