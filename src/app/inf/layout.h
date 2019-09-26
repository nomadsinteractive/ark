#ifndef ARK_APP_INF_LAYOUT_H_
#define ARK_APP_INF_LAYOUT_H_

#include <functional>
#include <vector>

#include "core/base/api.h"

#include "graphics/base/rect.h"
#include "graphics/forwarding.h"

#include "app/forwarding.h"
#include "app/view/layout_param.h"

namespace ark {

class ARK_API Layout {
public:
    virtual ~Layout() = default;

    struct Context {
        typedef std::function<std::vector<sp<LayoutParam>>()> LayoutParamDescriptor;

        Context(const LayoutParam& layoutParam, LayoutParamDescriptor lpd)
            : _client_width(layoutParam.contentWidth()), _content_width(0), _client_height(layoutParam.contentHeight()), _content_height(0),
              _layout_param_descriptor(std::move(lpd)) {
        }

        float _client_width;
        float _content_width;
        float _client_height;
        float _content_height;

        LayoutParamDescriptor _layout_param_descriptor;
    };

    virtual void begin(Context& ctx, LayoutParam& layoutParam) = 0;
    virtual Rect place(Context& ctx, LayoutParam& layoutParam) = 0;
    virtual Rect end(Context& ctx) = 0;
};

}

#endif
