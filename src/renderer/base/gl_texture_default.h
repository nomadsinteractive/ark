#ifndef ARK_RENDERER_BASE_GL_TEXTURE_DEFAULT_H_
#define ARK_RENDERER_BASE_GL_TEXTURE_DEFAULT_H_

#include <map>

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/types/global.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/block.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_texture.h"

namespace ark {

class ARK_API GLTextureDefault : public GLTexture {
public:
    GLTextureDefault(const sp<GLRecycler>& recycler, const sp<Size>& size, Format format, Feature features, const sp<Variable<sp<Bitmap>>>& bitmap);

//  [[plugin::resource-loader("texture")]]
    class BUILDER : public Builder<GLTexture> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<GLTexture> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;

        BeanFactory _factory;
        document _manifest;
        sp<Builder<String>> _src;
        Format _format;
        Feature _features;
    };

//  [[plugin::resource-loader::by-value("texture")]]
    class DICTIONARY : public Builder<GLTexture> {
    public:
        DICTIONARY(BeanFactory& parent, const String& value, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<GLTexture> build(const sp<Scope>& args) override;

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