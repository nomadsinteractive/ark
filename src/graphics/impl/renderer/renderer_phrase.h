#pragma once

#include "core/base/api.h"
#include "core/collection/list.h"
#include "core/inf/builder.h"

#include "graphics/inf/renderer.h"
#include "graphics/util/renderer_type.h"

namespace ark {

class ARK_API RendererPhrase : public Renderer, public Renderer::Group {
public:

    virtual void render(RenderRequest& renderRequest, const V3& position) override;
    virtual void addRenderer(sp<Renderer> renderer, const Traits& traits) override;

    void add(RendererType::Phrase phrase, sp<Renderer> renderer, sp<Boolean> discarded = nullptr, sp<Boolean> visible = nullptr);

private:
    DVList<sp<Renderer>> _phrases[RendererType::PHRASE_COUNT];

};

}
