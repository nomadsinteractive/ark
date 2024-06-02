#pragma once

#include "core/base/api.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API Renderer {
public:
    virtual ~Renderer() = default;

    virtual void render(RenderRequest& renderRequest, const V3& position) = 0;

    class ARK_API Group {
    public:
        virtual ~Group() = default;

        virtual void addRenderer(sp<Renderer> renderer, const Traits& traits) = 0;
    };

};

}
