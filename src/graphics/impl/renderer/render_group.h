#ifndef ARK_GRAPHICS_IMPL_RENDERER_RENDER_GROUP_H_
#define ARK_GRAPHICS_IMPL_RENDERER_RENDER_GROUP_H_

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/collection/list.h"
#include "core/inf/builder.h"

#include "graphics/inf/renderer.h"

namespace ark {

class ARK_API RendererGroup : public Renderer, public Renderer::Group {
public:
    ~RendererGroup() override;

    virtual void addRenderer(const sp<Renderer>& renderer) override;
    virtual void render(RenderRequest& renderRequest, const V3& position) override;

//  [[plugin::builder("render-group")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& beanFactory, const document& manifest);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        void loadGroup(RendererGroup& rendererGroup, const Scope& args);

    private:
        BeanFactory _factory;
        document _manifest;
    };

private:
    DVList<Renderer> _items;

};

}

#endif
