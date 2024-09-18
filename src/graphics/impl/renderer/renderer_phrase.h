#pragma once

#include <vector>

#include "core/base/api.h"
#include "core/collection/list.h"
#include "core/inf/builder.h"

#include "graphics/inf/renderer.h"
#include "graphics/util/renderer_type.h"

namespace ark {

class ARK_API RendererPhrase final : public Renderer, public Renderer::Group {
public:

    void render(RenderRequest& renderRequest, const V3& position) override;
    void addRenderer(sp<Renderer> renderer, const Traits& traits) override;

    void add(RendererType::Phrase phrase, sp<Renderer> renderer, sp<Boolean> discarded = nullptr, sp<Boolean> visible = nullptr);

//  [[plugin::builder]]
    class BUILDER final : public Builder<RendererPhrase> {
    public:
        BUILDER(BeanFactory& beanFactory, const document& manifest);

        sp<RendererPhrase> build(const Scope& args) override;

    private:
        struct Phrase;
        std::vector<Phrase> _phrases[RendererType::PHRASE_COUNT + 1];
    };

//  [[plugin::builder("render-group")]]
    class BUILDER_RENDERER final : public Builder<Renderer> {
    public:
        BUILDER_RENDERER(BeanFactory& beanFactory, const document& manifest);

        sp<Renderer> build(const Scope& args) override;

    private:
        BUILDER _impl;
    };

private:
    DVList<sp<Renderer>> _phrases[RendererType::PHRASE_COUNT];
};

}
