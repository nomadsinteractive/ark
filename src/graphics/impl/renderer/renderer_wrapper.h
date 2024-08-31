#pragma once

#include "core/forwarding.h"
#include "core/base/wrapper.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

namespace ark {

class RendererWrapper : public Renderer, public Wrapper<Renderer> {
public:
    RendererWrapper(const sp<Renderer>& delegate);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

};

}
