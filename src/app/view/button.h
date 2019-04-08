#ifndef ARK_APP_VIEW_BUTTON_H_
#define ARK_APP_VIEW_BUTTON_H_

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"
#include "core/types/owned_ptr.h"

#include "graphics/inf/renderer.h"
#include "graphics/forwarding.h"

#include "app/inf/event_listener.h"
#include "app/view/view.h"
#include "app/forwarding.h"

namespace ark {

class Button final : public View, public Renderer {
public:
    Button(const sp<Renderer>& foreground, const sp<Renderer>& background, const sp<Size>& block, Gravity gravity);
    ~Button() override;

    virtual void render(RenderRequest& renderRequest, float x, float y) override;

    void setForeground(View::State status, const sp<Renderer>& foreground);
    void setBackground(View::State status, const sp<Renderer>& background);

//  [[plugin::builder]]
    class BUILDER : public Builder<Button> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Button> build(const sp<Scope>& args) override;

    private:
        void loadStatus(const sp<Button>& button, const document& doc, BeanFactory& args, const sp<Scope>&);

    private:
        BeanFactory _factory;
        document _manifest;
        SafePtr<Builder<Renderer>> _foreground, _background;
        SafePtr<Builder<Size>> _size;
        String _gravity;
    };

//  [[plugin::builder("button")]]
    class BUILDER_IMPL2 : public Builder<Renderer> {
    public:
        BUILDER_IMPL2(BeanFactory& parent, const document& doc);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        BUILDER _delegate;
    };

private:
    virtual bool fireOnPush() override;
    virtual bool fireOnRelease() override;

private:
    op<RendererWithState> _foreground;
    op<RendererWithState> _background;
    Gravity _gravity;


};

}

#endif
