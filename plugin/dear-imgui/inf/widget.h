#ifndef ARK_PLUGIN_DEAR_IMGUI_INF_WIDGET_H_
#define ARK_PLUGIN_DEAR_IMGUI_INF_WIDGET_H_

#include "dear-imgui/api.h"

namespace ark {
namespace plugin {
namespace dear_imgui {

// [[script::bindings::auto]]
class ARK_PLUGIN_DEAR_IMGUI_API Widget {
public:
    virtual ~Widget() = default;

    virtual void render() = 0;
};

}
}
}


#endif
