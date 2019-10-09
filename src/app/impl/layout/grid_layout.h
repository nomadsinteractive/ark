#ifndef ARK_APP_IMPL_LAYOUT_GRID_LAYOUT_H_
#define ARK_APP_IMPL_LAYOUT_GRID_LAYOUT_H_

#include "core/inf/builder.h"

#include "app/forwarding.h"
#include "app/inf/layout.h"
#include "app/view/view.h"

namespace ark {

class GridLayout : public Layout {
public:
    GridLayout(uint32_t rows, uint32_t cols, const LayoutParam::Gravity gravity);

    virtual void begin(Context& ctx, LayoutParam& layoutParam) override;
    virtual Rect place(Context& ctx, LayoutParam& layoutParam) override;
    virtual Rect end(Context& ctx) override;

//  [[plugin::builder("grid")]]
    class BUILDER : public Builder<Layout> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Layout> build(const Scope& args) override;

    private:
        sp<Builder<Numeric>> _rows, _cols;
        LayoutParam::Gravity _gravity;

    };

private:
    uint32_t _rows, _cols;
    LayoutParam::Gravity _gravity;

    float _grid_width, _grid_height;
    uint32_t _flow_index;
};

}

#endif
