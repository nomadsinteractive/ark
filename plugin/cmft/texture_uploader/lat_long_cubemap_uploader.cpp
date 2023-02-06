#include "cmft/texture_uploader/lat_long_cubemap_uploader.h"

#include <cmft/cubemapfilter.h>

#include "core/inf/array.h"
#include "core/types/global.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"


namespace ark {
namespace plugin {
namespace cmft {

LatLongCubemapUploader::LatLongCubemapUploader(sp<Bitmap> bitmap, sp<Size> size)
    : _bitmap(std::move(bitmap)), _size(std::move(size))
{
}

void LatLongCubemapUploader::initialize(GraphicsContext& graphicsContext, Texture::Delegate& delegate)
{
    DCHECK(_size->widthAsFloat() == _size->heightAsFloat(), "Cubemap should be square, but (%.2f, %.2f) provided", _size->widthAsFloat(), _size->heightAsFloat());
    DCHECK(delegate.type() == Texture::TYPE_CUBEMAP, "This uploader uploads bitmaps to a cubmap, not Texture::Type(%d)", delegate.type());
    DCHECK_WARN(_bitmap->width() == _bitmap->height() * 2, "LatLong bitmap's size should be (2n, n), but got (%d, %d)", _bitmap->width(), _bitmap->height());

    ::cmft::Image input;
    input.m_data = _bitmap->bytes()->buf();
    input.m_width = _bitmap->width();
    input.m_height = _bitmap->height();
    input.m_dataSize = _bitmap->bytes()->size();
    input.m_format = ::cmft::TextureFormat::RGB32F;
    input.m_numMips = 1;
    input.m_numFaces = 1;

    uint32_t n = static_cast<uint32_t>(_size->widthAsFloat());

    ::cmft::Image output;
    ::cmft::imageCubemapFromLatLong(output, input);

    ::cmft::Image faceList[6];
    ::cmft::imageFaceListFromCubemap(faceList, output);

    Bitmap::Util<float>::rotate(reinterpret_cast<float*>(faceList[2].m_data), faceList[2].m_width, faceList[2].m_height, 3, 270);
    Bitmap::Util<float>::hvflip(reinterpret_cast<float*>(faceList[3].m_data), faceList[3].m_width, faceList[3].m_height, 3);
    Bitmap::Util<float>::hflip(reinterpret_cast<float*>(faceList[3].m_data), faceList[3].m_width, faceList[3].m_height, 3);

    const uint32_t imageFaceIndices[6] = {4, 5, 2, 3, 1, 0};
    std::vector<sp<ByteArray>> imagedata;
    for(uint32_t i = 0; i < 6; ++i)
    {
        ::cmft::Image& face = faceList[imageFaceIndices[i]];
        ::cmft::imageResize(face, n, n);
        imagedata.push_back(sp<ByteArray::Borrowed>::make(reinterpret_cast<uint8_t*>(face.m_data), face.m_dataSize));
    }
    delegate.uploadBitmap(graphicsContext, Bitmap(n, n, n * 4 * 3, 3, false), imagedata);

    for(uint32_t i = 0; i < 6; ++i)
        ::cmft::imageUnload(faceList[i]);
    ::cmft::imageUnload(output);
}

LatLongCubemapUploader::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _bitmap(factory.ensureConcreteClassBuilder<Bitmap>(manifest, Constants::Attributes::BITMAP)), _size(factory.ensureConcreteClassBuilder<Size>(manifest, Constants::Attributes::SIZE))
{
}

sp<Texture::Uploader> LatLongCubemapUploader::BUILDER::build(const Scope& args)
{
    return sp<LatLongCubemapUploader>::make(_bitmap->build(args), _size->build(args));
}

}
}
}
