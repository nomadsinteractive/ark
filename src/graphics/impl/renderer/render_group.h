#pragma once

#include <map>
#include <vector>

#include "core/base/api.h"
#include "core/collection/list.h"
#include "core/inf/builder.h"

#include "graphics/inf/renderer.h"
#include "graphics/util/renderer_type.h"

namespace ark {

class ARK_API RenderGroup final : public Renderer, public Renderer::Group {
public:

    void render(RenderRequest& renderRequest, const V3& position) override;
    void addRenderer(sp<Renderer> renderer, const Traits& traits) override;

    void add(RendererType::Priority priority, sp<Renderer> renderer, sp<Boolean> discarded = nullptr, sp<Boolean> visible = nullptr);

//  [[plugin::builder]]
    class BUILDER final : public Builder<RenderGroup> {
    public:
        BUILDER(BeanFactory& beanFactory, const document& manifest);

        sp<RenderGroup> build(const Scope& args) override;

    private:
        struct Phrase;
        std::vector<Phrase> _phrases;
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
    std::map<RendererType::Priority, DVList<sp<Renderer>>> _phrases;
};

}
