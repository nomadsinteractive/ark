#pragma once

#include "dear-imgui/api.h"

namespace ark::plugin::dear_imgui {

class ARK_PLUGIN_DEAR_IMGUI_API Widget {
public:
    virtual ~Widget() = default;

    virtual void render() = 0;
};

}
