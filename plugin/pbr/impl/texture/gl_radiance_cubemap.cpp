#include "impl/texture/gl_radiance_cubemap.h"

#include <cmft/clcontext.h>
#include <cmft/cubemapfilter.h>

#include "core/types/global.h"
#include "core/util/math.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

#include "renderer/base/render_controller.h"
#include "renderer/base/shader.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/texture.h"

namespace ark {

namespace {

struct OpenCLContext {
    OpenCLContext() {
        int ec = cmft::clLoad();
        WARN(ec != 0, "Loading OpenCL contexting failed.");
        if(ec != 0)
            cl_context = cmft::clInit();
    }
    ~OpenCLContext() {
        cmft::clDestroy(cl_context);
        cmft::clUnload();
    }

    cmft::ClContext* cl_context;
};

class TextureUploaderRadianceCubemap : public Texture::Uploader {
public:
    TextureUploaderRadianceCubemap(const sp<Texture>& texture, const sp<Size>& size)
        : _texture(texture), _size(size) {
    }

    virtual void upload(GraphicsContext& graphicsContext, Texture::Delegate& delegate) override {
        DCHECK(_size->width() == _size->height(), "Cubemap should be square, but (%.2f, %.2f) provided", _size->width(), _size->height());

        uint32_t tw = static_cast<uint32_t>(_texture->width());
        uint32_t th = static_cast<uint32_t>(_texture->height());

        cmft::Image input;
        cmft::imageCreate(input, tw, th, 0, 1, 1, cmft::TextureFormat::RGBA32F);

        if(!_texture->id())
            _texture->upload(graphicsContext, nullptr);

        Bitmap bitmap(tw, th, tw * 4 * sizeof(float), 4, sp<ByteArray::Borrowed>::make(reinterpret_cast<uint8_t*>(input.m_data), input.m_dataSize));
        _texture->delegate()->download(graphicsContext, bitmap);

        uint32_t n = static_cast<uint32_t>(_size->width());
        cmft::imageResize(input, n * 2, n);

        Bitmap::Util::hflip<float>(reinterpret_cast<float*>(input.m_data), n * 2, n, 4);
        cmft::imageToCubemap(input);

        const Global<OpenCLContext> clContext;
        cmft::Image output;
        cmft::imageCreate(output, n, n, 0, 1, 6, cmft::TextureFormat::RGBA32F);
        cmft::imageRadianceFilter(output, n, cmft::LightingModel::BlinnBrdf, false, 1, 8, 1, input, cmft::EdgeFixup::None, 6, clContext->cl_context);

        cmft::Image faceList[6];
        cmft::imageFaceListFromCubemap(faceList, output);

        const uint32_t imageFaceIndices[6] = {4, 5, 2, 3, 1, 0};

        Bitmap::Util::rotate<float>(reinterpret_cast<float*>(faceList[2].m_data), n, n, 4, 270);
        Bitmap::Util::hvflip<float>(reinterpret_cast<float*>(faceList[3].m_data), n, n, 4);
        Bitmap::Util::hflip<float>(reinterpret_cast<float*>(faceList[3].m_data), n, n, 4);

        for(uint32_t i = 0; i < 6; ++i)
        {
            const Bitmap bitmap(n, n, n * 4 * 4, 4, sp<ByteArray::Borrowed>::make(reinterpret_cast<uint8_t*>(faceList[imageFaceIndices[i]].m_data), faceList[imageFaceIndices[i]].m_dataSize));
            delegate.upload(graphicsContext, i, bitmap);
            LOGD("GLCubemap Uploaded, id = %d, width = %d, height = %d", static_cast<int32_t>(delegate.id()), n, n);
        }

        cmft::imageUnload(input);
        cmft::imageUnload(output);
    }

private:
    sp<Texture> _texture;
    sp<Size> _size;
};

}

GLRadianceCubemap::GLRadianceCubemap(const sp<RenderController>& renderController, const sp<Texture::Parameters>& parameters, const sp<Texture>& texture, const sp<Size>& size)
    : GLCubemap(renderController->recycler(), size, parameters, sp<TextureUploaderRadianceCubemap>::make(texture, size))
{
}

GLRadianceCubemap::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _render_controller(resourceLoaderContext->renderController()), _size(factory.ensureConcreteClassBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _texture(factory.ensureBuilder<Texture>(manifest, Constants::Attributes::TEXTURE)), _parameters(sp<Texture::Parameters>::make(manifest))
{
}

sp<Texture> GLRadianceCubemap::BUILDER::build(const sp<Scope>& args)
{
    const sp<Size> size = _size->build(args);
    const sp<GLRadianceCubemap> cubemap = sp<GLRadianceCubemap>::make(_render_controller, _parameters, _texture->build(args), size);
    return _render_controller->createResource<Texture>(size, sp<Variable<sp<Texture::Delegate>>::Const>::make(cubemap), Texture::TYPE_CUBEMAP);
}

}
