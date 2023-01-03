#include "renderer/base/bitmap_bundle.h"

#include "graphics/base/bitmap.h"

#include "renderer/base/atlas.h"


namespace ark {

BitmapBundle::BitmapBundle(const Atlas& atlas, sp<Bitmap> atlasBitmap)
    : _atlas(atlas), _atlas_bitmap(std::move(atlasBitmap))
{
}

sp<Bitmap> BitmapBundle::getBitmap(int32_t resid)
{
    const auto iter = _bitmaps.find(resid);
    if(iter != _bitmaps.end())
        return iter->second;

    const Atlas::Item& item = _atlas.at(resid);
    const V2& size = item.size();
    const V2& pivot = item.pivot();
    const Rect& bounds = item.bounds();
    const uint32_t width = static_cast<uint32_t>(size.x());
    const uint32_t height = static_cast<uint32_t>(size.y());
    Bitmap b(width, height, width * _atlas_bitmap->channels(), _atlas_bitmap->channels(), true);
    memset(b.at(0, 0), 0, b.rowBytes() * b.height());
    const Rect position = _atlas.getItemBounds(resid);
    const int32_t ox = static_cast<int32_t>((bounds.left() + pivot.x()) * size.x());
    const int32_t oy = static_cast<int32_t>((bounds.top() + pivot.y()) * size.y());
    const uint32_t w = static_cast<uint32_t>(bounds.width() * size.x());
    const uint32_t h = static_cast<uint32_t>(bounds.height() * size.y());
    const uint32_t uvLeft = static_cast<uint32_t>(position.left());
    const uint32_t uvTop = static_cast<uint32_t>(position.top());
    const uint32_t stride = _atlas_bitmap->componentSize() * w;
    for(uint32_t i = 0; i < h; ++i)
        b.draw(ox, static_cast<int32_t>(i) + oy, _atlas_bitmap->at(uvLeft, uvTop + i), w, 1, stride);

    sp<Bitmap>& bitmap = _bitmaps[resid];
    bitmap = sp<Bitmap>::make(std::move(b));
    return bitmap;
}

}
