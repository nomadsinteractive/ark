#pragma once

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/collection/list.h"
#include "core/inf/builder.h"

#include "graphics/inf/renderer.h"

namespace ark {

class ARK_API [[deprecated]] RendererGroup final : public Renderer, public Renderer::Group {
public:
    ~RendererGroup() override;

    void render(RenderRequest& renderRequest, const V3& position) override;
    void addRenderer(sp<Renderer> renderer, const Traits& traits) override;

    void add(sp<Renderer> renderer, sp<Boolean> discarded = nullptr, sp<Boolean> visible = nullptr);

private:
    DVList<sp<Renderer>> _renderers;
};

}
