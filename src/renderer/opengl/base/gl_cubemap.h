#ifndef ARK_RENDERER_BASE_GL_CUBEMAP_H_
#define ARK_RENDERER_BASE_GL_CUBEMAP_H_

#include <map>

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/types/global.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/opengl/base/gl_texture.h"

namespace ark {

class ARK_API GLCubemap : public GLTexture {
public:
    GLCubemap(const sp<GLRecycler>& recycler, const sp<Size>& size, const sp<Texture::Parameters>& parameters, std::vector<sp<Variable<bitmap>>> bitmaps);

//  [[plugin::resource-loader("cubemap")]]
    class BUILDER : public Builder<Texture> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Texture> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;

        BeanFactory _factory;
        document _manifest;
        sp<Builder<Size>> _size;
        sp<Builder<String>> _srcs[6];
        sp<Texture::Parameters> _parameters;
    };

protected:
    virtual void doPrepareTexture(GraphicsContext& graphicsContext, uint32_t id) override;

private:
    std::vector<sp<Variable<bitmap>>> _bitmaps;
};

}

#endif
