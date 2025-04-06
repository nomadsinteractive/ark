#pragma once

#include "core/base/string.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/renderer.h"
#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class FPSCounter : public Renderer {
public:
[[deprecated]]
    FPSCounter(const sp<Clock>& clock, float refreshInterval, const sp<Text>& characters, const String& message);

    virtual void render(RenderRequest& renderRequest, const V3& position, const sp<DrawDecorator>& drawDecorator) override;

    void updateFPS(float fps);

//  [[plugin::builder("fps-counter")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Text>> _characters;
        String _message;
        float _interval;
    };

private:
    sp<Text> _characters;
    String _message;

    uint32_t _frame_rendered;
    sp<Numeric> _duration;
    float _refresh_interval;
    float _last_refresh;

};

}
