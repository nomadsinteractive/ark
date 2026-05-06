#pragma once

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/collection/list.h"
#include "core/inf/builder.h"
#include "core/util/documents.h"

#include "graphics/inf/renderer.h"
#include "graphics/util/renderer_type.h"

namespace ark {

class ARK_API RenderGroup final : public Renderer, public Renderer::Group {
public:

    void render(RenderRequest& renderRequest, const V3& position, const sp<DrawDecorator>& drawDecorator) override;
    void addRenderer(sp<Renderer> renderer, const Traits& traits) override;

    void add(RendererType::Priority priority, sp<Renderer> renderer, sp<Boolean> discarded = nullptr, sp<Boolean> visible = nullptr);

//  [[plugin::builder]]
    class BUILDER final : public Builder<RenderGroup> {
    public:
        BUILDER(BeanFactory& beanFactory, const document& manifest);

        sp<RenderGroup> build(const Scope& args) override;

    private:
        struct RendererBuilder {
            RendererBuilder(BeanFactory& beanFactory, const document& manifest)
                : _renderer(beanFactory.ensureBuilder<Renderer>(manifest)), _priority(Documents::getAttribute<RendererType::Priority>(manifest, "priority", RendererType::PRIORITY_DEFAULT))
            {
            }

            sp<Builder<Renderer>> _renderer;
            RendererType::Priority _priority;
        };

        Vector<RendererBuilder> _renderers;
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
    Map<RendererType::Priority, DV_FList<sp<Renderer>>> _renderers;
};

}
