#ifndef ARK_PLUGIN_PBR_IMPL_TEXTURE_GL_RANDIANCE_CUBEMAP_H_
#define ARK_PLUGIN_PBR_IMPL_TEXTURE_GL_RANDIANCE_CUBEMAP_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/opengl/base/gl_cubemap.h"

namespace ark {

class GLRadianceCubemap : public GLCubemap {
public:
    GLRadianceCubemap(const sp<RenderController>& renderController, const sp<Texture::Parameters>& parameters, const sp<Texture>& texture, const sp<Size>& size);

//  [[plugin::resource-loader("radiance-cubemap")]]
    class BUILDER : public Builder<Texture> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Texture> build(const sp<Scope>& args) override;

    private:
        sp<RenderController> _render_controller;
        document _manifest;
        sp<Builder<Size>> _size;
        sp<Builder<Texture>> _texture;
        sp<Texture::Parameters> _parameters;
    };

};

}

#endif
