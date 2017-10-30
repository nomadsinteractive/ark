#ifndef ARK_APP_IMPL_LAYOUT_VERTICAL_LAYOUT_H_
#define ARK_APP_IMPL_LAYOUT_VERTICAL_LAYOUT_H_

#include "core/inf/builder.h"

#include "app/forwarding.h"
#include "app/inf/layout.h"
#include "app/view/view.h"

namespace ark {

class VerticalLayout : public Layout {
public:
    VerticalLayout(const View::Gravity gravity);

    virtual void begin(LayoutParam& layoutParam) override;
    virtual Rect place(LayoutParam& layoutParam) override;
    virtual Rect end() override;

//  [[plugin::builder("vertical")]]
    class BUILDER : public Builder<Layout> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Layout> build(const sp<Scope>& args) override;

    private:
        View::Gravity _gravity;

    };

private:
    float _content_width, _content_height;
    View::Gravity _place_gravity, _end_gravity;
    float _flowy;
    float _max_width;
};

}

#endif
