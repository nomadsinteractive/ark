#include "impl/texture/irradiance_cubemap_uploader.h"

#include <cmft/cubemapfilter.h>

#include "core/inf/array.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

namespace ark {

IrradianceCubemapUploader::IrradianceCubemapUploader(const sp<Texture>& texture, const sp<Size>& size)
    : _texture(texture), _size(size)
{
}

void IrradianceCubemapUploader::upload(GraphicsContext& graphicsContext, Texture::Delegate& delegate)
{
    DCHECK(_size->width() == _size->height(), "Cubemap should be square, but (%.2f, %.2f) provided", _size->width(), _size->height());
    DCHECK(delegate.type() == Texture::TYPE_CUBEMAP, "This uploader uploads bitmaps to a cubmap, not Texture::Type(%d)", delegate.type());

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
    }

    cmft::imageUnload(input);
    cmft::imageUnload(output);
}

IrradianceCubemapUploader::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _size(factory.ensureConcreteClassBuilder<Size>(manifest, Constants::Attributes::SIZE)), _texture(factory.ensureBuilder<Texture>(manifest, Constants::Attributes::TEXTURE))
{
}

sp<Texture::Uploader> IrradianceCubemapUploader::BUILDER::build(const sp<Scope>& args)
{
    return sp<IrradianceCubemapUploader>::make(_texture->build(args), _size->build(args));
}

}