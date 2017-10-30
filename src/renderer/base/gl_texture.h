#ifndef ARK_RENDERER_BASE_GL_TEXTURE_H_
#define ARK_RENDERER_BASE_GL_TEXTURE_H_

#include <map>

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/types/global.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/block.h"

#include "renderer/forwarding.h"
#include "renderer/inf/gl_resource.h"

namespace ark {

//[[core::class]]
class ARK_API GLTexture : public GLResource {
public:
    enum Format {
        FORMAT_AUTO = 256,
        FORMAT_ALPHA = 0,
        FORMAT_RG = 1,
        FORMAT_RGB = 2,
        FORMAT_RGBA = 3,
        FORMAT_SIGNED = 4,
        FORMAT_INTEGRAL = 8,
    };

    GLTexture(const sp<GLRecycler>& _recycler, uint32_t width, uint32_t height, const sp<Variable<bitmap>>& bitmap, Format format = FORMAT_AUTO);
    ~GLTexture();

    virtual uint32_t id() override;
    virtual void prepare(GraphicsContext& graphicsContext) override;
    virtual void recycle(GraphicsContext&) override;

//  [[script::bindings::property]]
    uint32_t width() const;
//  [[script::bindings::property]]
    uint32_t height() const;

//  [[script::bindings::auto]]
    sp<Bitmap> getBitmap() const;

    void active(const sp<GLProgram>& program, uint32_t texture) const;

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<GLTexture> {
    public:
        BUILDER(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<GLTexture> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;

        BeanFactory _factory;
        document _manifest;
        sp<Builder<String>> _src;
        Format _format;
    };


//  [[plugin::resource-loader::by-value]]
    class DICTIONARY : public Builder<GLTexture> {
    public:
        DICTIONARY(BeanFactory& parent, const String& value, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<GLTexture> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        String _src;
    };


private:
    class Recycler : public GLResource {
    public:
        Recycler(uint32_t id);

        virtual uint32_t id() override;
        virtual void prepare(GraphicsContext&) override;
        virtual void recycle(GraphicsContext&) override;

    private:
        uint32_t _id;
    };

    void setTexParameters(const document& doc);
    void setTexParameter(uint32_t name, int32_t value);
    void setTexFormat(Format format);

    friend class BUILDER;
    friend class DICTIONARY;

private:
    sp<GLRecycler> _recycler;

    uint32_t _id;
    uint32_t _width;
    uint32_t _height;
    sp<Variable<sp<Bitmap>>> _bitmap;
    Format _format;

    std::map<uint32_t, int32_t> _tex_parameters;

    Global<GLConstants> _gl_constants;

};

}

#endif
