#include "app/base/application_resource.h"

#include "core/base/string.h"
#include "core/inf/dictionary.h"

#include "graphics/impl/resource/image_resource.h"
#include "graphics/impl/resource/png_resource.h"
#include "graphics/impl/resource/jpeg_resource.h"
#include "graphics/impl/resource/stb_image_resource.h"

#include "renderer/base/gl_resource_manager.h"

namespace ark {

ApplicationResource::ApplicationResource(const sp<Dictionary<document>>& documents, const sp<Asset>& images)
    : _images(images), _documents(documents), _bitmap_loader(createImageLoader(false)), _bitmap_bounds_loader(createImageLoader(true)), _gl_resources(createGLResourceManager())
{
}

const sp<GLResourceManager>& ApplicationResource::glResourceManager() const
{
    return _gl_resources;
}

const sp<Dictionary<document>>& ApplicationResource::documents() const
{
    return _documents;
}

document ApplicationResource::loadDocument(const String& name) const
{
    return _documents->get(name);
}

bitmap ApplicationResource::loadBitmap(const String& name) const
{
    return _bitmap_loader->get(name);
}

bitmap ApplicationResource::loadBitmapBounds(const String& name) const
{
    return _bitmap_bounds_loader->get(name);
}

sp<Dictionary<bitmap>> ApplicationResource::createImageLoader(bool justDecodeBounds) const
{
#ifdef ARK_USE_STB_IMAGE
    const sp<ImageResource> imageResource = sp<ImageResource>::make(sp<STBImageResource>::make(_images, justDecodeBounds));
#else
    const sp<ImageResource> imageResource = sp<ImageResource>::make(nullptr);
#endif
    imageResource->addLoader("png", sp<PNGResource>::make(_images, justDecodeBounds));
#ifdef ARK_USE_LIBJPEG_TURBO
    const sp<JPEGResource> jpegResource = sp<JPEGResource>::make(_images, justDecodeBounds);
    imageResource->addLoader("jpg", jpegResource);
    imageResource->addLoader("jpeg", jpegResource);
#endif
    return imageResource;
}

sp<GLResourceManager> ApplicationResource::createGLResourceManager() const
{
    return sp<GLResourceManager>::adopt(new GLResourceManager(_bitmap_loader, _bitmap_bounds_loader));
}

}
