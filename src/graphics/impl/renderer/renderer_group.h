#ifndef ARK_GRAPHICS_IMPL_RENDERER_RENDERER_GROUP_H_
#define ARK_GRAPHICS_IMPL_RENDERER_RENDERER_GROUP_H_

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/collection/filtered_list.h"
#include "core/inf/builder.h"

#include "graphics/inf/renderer.h"

namespace ark {

class ARK_API RendererGroup : public Renderer, public Renderer::Group {
public:
    ~RendererGroup();

    virtual void addRenderer(const sp<Renderer>& renderer) override;
    virtual void render(RenderRequest& renderRequest, float x, float y) override;

    void loadGroup(const document& manifest, BeanFactory& factory, const sp<Scope>& args);

//  [[plugin::builder("renderer-group")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& beanFactory, const document& manifest);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        BeanFactory _bean_factory;
        document _manifest;
    };

private:
    DisposableItemList<Renderer> _items;

};

}

#endif
