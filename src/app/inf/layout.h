#ifndef ARK_APP_INF_LAYOUT_H_
#define ARK_APP_INF_LAYOUT_H_

#include <functional>
#include <vector>

#include "core/base/api.h"
#include "core/types/safe_var.h"

#include "graphics/base/rect.h"
#include "graphics/forwarding.h"

#include "app/forwarding.h"
#include "app/view/view.h"

namespace ark {

class ARK_API Layout {
public:
    virtual ~Layout() = default;

    struct Context {
        typedef std::function<std::vector<sp<LayoutParam>>()> LayoutParamDescriptor;

        Context(const LayoutParam& layoutParam, LayoutParamDescriptor lpd)
            : _client_width(layoutParam.contentWidth()), _client_height(layoutParam.contentHeight()), _layout_param_descriptor(std::move(lpd)) {
        }

        float _client_width;
        float _client_height;

        LayoutParamDescriptor _layout_param_descriptor;
    };

    virtual void begin(Context& ctx, LayoutParam& layoutParam) = 0;
    virtual Rect place(Context& ctx, LayoutParam& layoutParam) = 0;
    virtual Rect end(Context& ctx) = 0;
};

class ARK_API LayoutV2 {
public:
    virtual ~LayoutV2() = default;

    virtual V2 inflate(const std::vector<sp<LayoutParam>>& slots) = 0;
    virtual std::vector<V2> place(const std::vector<sp<LayoutParam>>& slots, const LayoutParam& parent, const V2& contentSize) = 0;

};

}

#endif
