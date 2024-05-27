#pragma once

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/implements.h"
#include "core/types/safe_ptr.h"
#include "core/types/owned_ptr.h"

#include "graphics/inf/renderer.h"
#include "graphics/forwarding.h"

#include "app/inf/event_listener.h"
#include "app/view/view.h"
#include "app/view/layout_param.h"
#include "app/forwarding.h"

namespace ark {

class Button final : public View, Implements<Button, View> {
public:
    Button(sp<Renderer> foreground, sp<Renderer> background, sp<Size> block, LayoutParam::Gravity gravity);
    ~Button() override;

    void setForeground(View::State status, sp<Renderer> foreground, const sp<Boolean>& enabled);
    void setBackground(View::State status, sp<Renderer> background, const sp<Boolean>& enabled);

//  [[plugin::builder]]
    class BUILDER : public Builder<Button> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Button> build(const Scope& args) override;

    private:
        void loadStatus(const sp<Button>& button, const document& doc, BeanFactory& args, const Scope&);

    private:
        BeanFactory _factory;
        document _manifest;
        SafePtr<Builder<Renderer>> _foreground, _background;
        SafePtr<Builder<Size>> _size;
        String _gravity;
    };

private:
    op<RendererWithState> _foreground;
    op<RendererWithState> _background;
    LayoutParam::Gravity _gravity;


};

}
