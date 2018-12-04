#ifndef ARK_RENDERER_BASE_GL_TEXTURE_2D_H_
#define ARK_RENDERER_BASE_GL_TEXTURE_2D_H_

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/block.h"

#include "renderer/forwarding.h"
#include "renderer/base/texture.h"

#include "renderer/opengl/base/gl_texture.h"

namespace ark {

class GLTexture2D : public GLTexture {
public:
    GLTexture2D(const sp<GLRecycler>& recycler, const sp<Size>& size, const sp<Texture::Parameters>& parameters, const sp<Variable<bitmap>>& bitmap);

//  [[plugin::resource-loader("texture")]]
    class BUILDER : public Builder<Texture> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Texture> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;

        BeanFactory _factory;
        document _manifest;
        SafePtr<Builder<String>> _src;
        sp<Texture::Parameters> _parameters;
    };

//  [[plugin::resource-loader::by-value("texture")]]
    class DICTIONARY : public Builder<Texture> {
    public:
        DICTIONARY(BeanFactory& parent, const String& value, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Texture> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        String _src;
    };

protected:
    virtual void doPrepareTexture(GraphicsContext& graphicsContext, uint32_t id) override;

private:
    sp<Variable<sp<Bitmap>>> _bitmap;
};

}

#endif
