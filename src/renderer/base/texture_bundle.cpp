#include "renderer/base/texture_bundle.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine.h"
#include "renderer/inf/renderer_factory.h"

namespace ark {

namespace {

class UploaderBitmapBundle : public Texture::Uploader {
public:
    UploaderBitmapBundle(const sp<Dictionary<bitmap>>& bitmapLoader, const String& name)
        : _bitmap_loader(bitmapLoader), _name(name) {
    }

    virtual void upload(GraphicsContext& graphicContext, Texture::Delegate& delegate) override {
        const bitmap bitmap = _bitmap_loader->get(_name);
        DCHECK(bitmap, "Texture resource \"%s\" not found", _name.c_str());
        delegate.uploadBitmap(graphicContext, 0, bitmap);
    }

private:
    sp<Dictionary<bitmap>> _bitmap_loader;
    String _name;
};

}

TextureBundle::TextureBundle(const sp<RenderController>& renderController)
    : _render_controller(renderController), _renderer_factory(_render_controller->_render_engine->rendererFactory()),
      _bitmap_loader(_render_controller->_bitmap_loader), _bitmap_bounds_loader(_render_controller->_bitmap_bounds_loader)
{
}

const sp<Texture>& TextureBundle::getTexture(const String& src)
{
    sp<Texture>& texture = _textures[src];
    if(texture)
        return texture;

    texture = doCreateTexture(src, sp<Texture::Parameters>::make(Texture::TYPE_2D));
    return texture;
}

const sp<Texture>& TextureBundle::createTexture(const String& src, const sp<Texture::Parameters>& parameters)
{
    sp<Texture>& texture = _textures[src];
    DWARN(!texture, "Overriding Texture \"%s\"", src.c_str());
    texture = doCreateTexture(src, parameters);
    return texture;
}

sp<Texture> TextureBundle::doCreateTexture(const String& src, const sp<Texture::Parameters>& parameters) const
{
    const bitmap bitmapBounds = _bitmap_bounds_loader->get(src);
    DCHECK(bitmapBounds, "Texture resource \"%s\" not found", src.c_str());
    sp<Texture> texture = _renderer_factory->createTexture(sp<Size>::make(bitmapBounds->width(), bitmapBounds->height()), parameters, sp<UploaderBitmapBundle>::make(_bitmap_loader, src));
    DCHECK(texture, "Texture \"%s\" not loaded", src.c_str());
    _render_controller->upload(texture, nullptr, RenderController::US_ONCE_AND_ON_SURFACE_READY);
    return texture;
}

}
