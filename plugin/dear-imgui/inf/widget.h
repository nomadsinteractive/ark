#ifndef ARK_PLUGIN_DEAR_IMGUI_INF_WIDGET_H_
#define ARK_PLUGIN_DEAR_IMGUI_INF_WIDGET_H_

namespace ark {
namespace plugin {
namespace dear_imgui {

class Widget {
public:
    virtual ~Widget() = default;

    virtual void render() = 0;
};

}
}
}


#endif
