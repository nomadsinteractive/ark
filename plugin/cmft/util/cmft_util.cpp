#include "cmft/util/cmft_util.h"

#include "core/inf/array.h"

#include "graphics/base/bitmap.h"

namespace ark {
namespace plugin {
namespace cmft {

::cmft::Image CMFTUtil::bitmapToImage(const Bitmap& bitmap)
{
    ::cmft::Image image;
    image.m_data = bitmap.bytes()->buf();
    image.m_width = bitmap.width();
    image.m_height = bitmap.height();
    image.m_dataSize = bitmap.bytes()->size();
    image.m_format = ::cmft::TextureFormat::RGB32F;
    image.m_numMips = 1;
    image.m_numFaces = 1;
    return image;
}

}
}
}
