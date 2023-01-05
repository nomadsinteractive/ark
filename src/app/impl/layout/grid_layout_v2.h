#ifndef ARK_APP_IMPL_LAYOUT_GRID_LAYOUT_V2_H_
#define ARK_APP_IMPL_LAYOUT_GRID_LAYOUT_V2_H_

#include "core/inf/builder.h"

#include "app/forwarding.h"
#include "app/inf/layout.h"
#include "app/view/layout_param.h"

namespace ark {

class GridLayoutV2 : public Layout {
public:
    GridLayoutV2(uint32_t rows, uint32_t cols, const LayoutParam::Gravity gravity);

    virtual V2 inflate(const std::vector<sp<LayoutParam>>& slots) override;
    virtual std::vector<V2> place(const std::vector<sp<LayoutParam>>& slots, const LayoutParam& parent, const V2& contentSize) override;

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

};

}

#endif
