#include "renderer/impl/resource/gl_texture_resource.h"

#include "graphics/base/bitmap.h"
#include "graphics/impl/variable/bitmap/resource_bitmap_variable.h"

#include "renderer/base/gl_texture.h"

namespace ark {

GLTextureResource::GLTextureResource(const sp<GLRecycler>& recycler, const sp<Dictionary<bitmap>>& bitmapLoader, const sp<Dictionary<bitmap>>& bitmapBoundsLoader)
    : _recycler(recycler), _bitmap_loader(bitmapLoader), _bitmap_bounds_loader(bitmapBoundsLoader)
{
}

sp<GLTexture> GLTextureResource::get(const String& name)
{
    const bitmap bitmap = _bitmap_bounds_loader->get(name);
    DCHECK(bitmap, "Texture resource \"%s\" not found", name.c_str());
    return sp<GLTexture>::make(_recycler, bitmap->width(), bitmap->height(), sp<ResourceBitmapVariable>::make(_bitmap_loader, name));
}

}
