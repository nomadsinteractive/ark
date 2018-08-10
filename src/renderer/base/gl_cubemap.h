#ifndef ARK_RENDERER_BASE_GL_CUBEMAP_H_
#define ARK_RENDERER_BASE_GL_CUBEMAP_H_

#include <map>

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/types/global.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_texture.h"

namespace ark {

class ARK_API GLCubemap : public GLTexture {
public:
    GLCubemap(const sp<GLRecycler>& recycler, const sp<Size>& size, GLTexture::Format format, GLTexture::Feature features, std::vector<sp<Variable<bitmap>>> bitmaps);

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<GLCubemap> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<GLCubemap> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;

        BeanFactory _factory;
        document _manifest;
        sp<Builder<Size>> _size;
        sp<Builder<String>> _srcs[6];
        GLTexture::Format _format;
        GLTexture::Feature _features;
    };

//  [[plugin::resource-loader("cubemap")]]
    class BUILDER_IMPL1 : public Builder<GLTexture> {
    public:
        BUILDER_IMPL1(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<GLTexture> build(const sp<Scope>& args) override;

    private:
        BUILDER _delegate;
    };

protected:
    virtual void doPrepareTexture(GraphicsContext& graphicsContext, uint32_t id) override;

private:
    std::vector<sp<Variable<bitmap>>> _bitmaps;
};

}

#endif
