#pragma once

#include "core/forwarding.h"
#include "core/base/wrapper.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

namespace ark {

class RendererWrapper final : public Renderer, public Wrapper<Renderer> {
public:
    RendererWrapper(sp<Renderer> delegate);

    void render(RenderRequest& renderRequest, const V3& position, const sp<DrawDecorator>& drawDecorator) override;
};

}
