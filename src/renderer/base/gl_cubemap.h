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
#include "renderer/inf/gl_resource.h"

namespace ark {

class ARK_API GLCubemap : public GLResource {
public:
    GLCubemap(const sp<GLRecycler>& recycler, uint32_t width, uint32_t height, const std::vector<sp<Variable<bitmap>>>& bitmaps, GLTexture::Format format = GLTexture::FORMAT_AUTO);
    ~GLCubemap();

    virtual uint32_t id() override;
    virtual void prepare(GraphicsContext& graphicsContext) override;
    virtual void recycle(GraphicsContext&) override;

    uint32_t width() const;
    uint32_t height() const;

    void active(const sp<GLProgram>& program, uint32_t id) const;

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
    };

private:
    void setTexParameters(const document& doc);
    void setTexParameter(uint32_t name, int32_t value);
    void setTexFormat(GLTexture::Format format);

    friend class BUILDER;

private:
    sp<GLRecycler> _recycler;

    uint32_t _id;
    uint32_t _width;
    uint32_t _height;
    std::vector<sp<Variable<bitmap>>> _bitmaps;
    GLTexture::Format _format;

    std::map<uint32_t, int32_t> _tex_parameters;

    Global<GLConstants> _gl_constants;

};

}

#endif
