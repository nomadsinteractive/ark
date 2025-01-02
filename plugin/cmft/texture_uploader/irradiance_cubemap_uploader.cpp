#include "cmft/texture_uploader/irradiance_cubemap_uploader.h"

#include <cmft/cubemapfilter.h>

#include "core/inf/array.h"

#include "graphics/base/bitmap.h"
#include "graphics/components/size.h"

#include "cmft/util/cmft_util.h"

namespace ark::plugin::cmft {

IrradianceCubemapUploader::IrradianceCubemapUploader(sp<Bitmap> bitmap, sp<Size> size)
    : _bitmap(std::move(bitmap)), _size(std::move(size))
{
}

void IrradianceCubemapUploader::initialize(GraphicsContext& graphicsContext, Texture::Delegate& delegate)
{
    DCHECK(_size->widthAsFloat() == _size->heightAsFloat(), "Cubemap should be square, but (%.2f, %.2f) provided", _size->widthAsFloat(), _size->heightAsFloat());
    DCHECK(delegate.type() == Texture::TYPE_CUBEMAP, "This uploader uploads bitmaps to a cubmap, not Texture::Type(%d)", delegate.type());

    ::cmft::Image input;
    ::cmft::imageConvert(input, ::cmft::TextureFormat::RGBA32F, CMFTUtil::bitmapToImage(_bitmap));

    uint32_t n = static_cast<uint32_t>(_size->widthAsFloat());
    ::cmft::Image output;
    ::cmft::imageCreate(output, n, n, 0, 1, 6, ::cmft::TextureFormat::RGBA32F);
    ::cmft::imageToCubemap(input);
    ::cmft::imageIrradianceFilterSh(output, n, input);

    ::cmft::Image faceList[6];
    ::cmft::imageFaceListFromCubemap(faceList, output);

    const uint32_t imageFaceIndices[6] = {4, 5, 2, 3, 1, 0};

    Bitmap::Util<float>::rotate(reinterpret_cast<float*>(faceList[2].m_data), n, n, 4, 270);
    Bitmap::Util<float>::hvflip(reinterpret_cast<float*>(faceList[3].m_data), n, n, 4);
    Bitmap::Util<float>::hflip(reinterpret_cast<float*>(faceList[3].m_data), n, n, 4);

    const Bitmap uploadingBitmap(n, n, n * 4 * 4, 4, false);
    std::vector<sp<ByteArray>> imagedata;
    for(uint32_t i = 0; i < 6; ++i)
        imagedata.push_back(sp<ByteArray::Borrowed>::make(reinterpret_cast<uint8_t*>(faceList[imageFaceIndices[i]].m_data), faceList[imageFaceIndices[i]].m_dataSize));
    delegate.uploadBitmap(graphicsContext, uploadingBitmap, imagedata);

    ::cmft::imageUnload(input);
    ::cmft::imageUnload(output);
}

IrradianceCubemapUploader::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _bitmap(factory.ensureConcreteClassBuilder<Bitmap>(manifest, constants::BITMAP)), _size(factory.ensureConcreteClassBuilder<Size>(manifest, constants::SIZE))
{
}

sp<Texture::Uploader> IrradianceCubemapUploader::BUILDER::build(const Scope& args)
{
    return sp<IrradianceCubemapUploader>::make(_bitmap->build(args), _size->build(args));
}

}
