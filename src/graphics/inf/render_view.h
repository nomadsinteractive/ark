#pragma once

#include "graphics/base/v4.h"

namespace ark {

class RenderView {
public:
    virtual ~RenderView() = default;

    virtual void onSurfaceCreated() = 0;
    virtual void onSurfaceChanged(uint32_t width, uint32_t height) = 0;
    virtual void onRenderFrame(V4 backgroundColor, RenderCommand& renderCommand) = 0;
};

}
