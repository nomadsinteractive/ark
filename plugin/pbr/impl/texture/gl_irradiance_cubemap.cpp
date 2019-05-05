#include "impl/texture/gl_irradiance_cubemap.h"

#include <cmft/cubemapfilter.h>

#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

#include "renderer/base/render_controller.h"
#include "renderer/base/shader.h"
#include "renderer/base/resource_loader_context.h"

namespace ark {

namespace {

class TextureUploaderIrradianceCubemap : public Texture::Uploader {
public:
    TextureUploaderIrradianceCubemap(const sp<Texture>& texture, const sp<Size>& size)
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
        cmft::Image output;
        cmft::imageCreate(output, n, n, 0, 1, 6, cmft::TextureFormat::RGBA32F);
        cmft::imageToCubemap(input);
        cmft::imageIrradianceFilterSh(output, n, input);

        cmft::Image faceList[6];
        cmft::imageFaceListFromCubemap(faceList, output);

        const uint32_t imageFaceIndices[6] = {4, 5, 2, 3, 1, 0};

        Bitmap::Util::rotate<float>(reinterpret_cast<float*>(faceList[2].m_data), n, n, 4, 270);
        Bitmap::Util::hvflip<float>(reinterpret_cast<float*>(faceList[3].m_data), n, n, 4);
        Bitmap::Util::hflip<float>(reinterpret_cast<float*>(faceList[3].m_data), n, n, 4);

        for(uint32_t i = 0; i < 6; ++i)
        {
            const Bitmap bitmap(n, n, n * 4 * 4, 4, sp<ByteArray::Borrowed>::make(reinterpret_cast<uint8_t*>(faceList[imageFaceIndices[i]].m_data), faceList[imageFaceIndices[i]].m_dataSize));
            delegate.uploadBitmap(graphicsContext, i, bitmap);
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


GLIrradianceCubemap::GLIrradianceCubemap(const sp<RenderController>& renderController, const sp<Texture::Parameters>& params, const sp<Texture>& texture, const sp<Size>& size)
    : GLCubemap(renderController->recycler(), size, params, sp<TextureUploaderIrradianceCubemap>::make(texture, size))
{
}

GLIrradianceCubemap::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _render_controller(resourceLoaderContext->renderController()), _size(factory.ensureConcreteClassBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _texture(factory.ensureBuilder<Texture>(manifest, Constants::Attributes::TEXTURE)), _parameters(sp<Texture::Parameters>::make(manifest))
{
}

sp<Texture> GLIrradianceCubemap::BUILDER::build(const sp<Scope>& args)
{
    const sp<Size> size = _size->build(args);
    const sp<GLIrradianceCubemap> cubemap = sp<GLIrradianceCubemap>::make(_render_controller, _parameters, _texture->build(args), _size->build(args));
    return _render_controller->createResource<Texture>(size, sp<Variable<sp<Texture::Delegate>>::Const>::make(cubemap), Texture::TYPE_CUBEMAP);
}

}
