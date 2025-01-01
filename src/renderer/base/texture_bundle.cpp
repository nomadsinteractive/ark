#include "renderer/base/texture_bundle.h"

#include "graphics/base/bitmap.h"
#include "graphics/traits/size.h"

#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine.h"
#include "renderer/inf/renderer_factory.h"

namespace ark {

namespace {

class UploaderBitmapBundle final : public Texture::Uploader {
public:
    UploaderBitmapBundle(const sp<Dictionary<bitmap>>& bitmapLoader, const String& name)
        : _bitmap_loader(bitmapLoader), _name(name) {
    }

    void initialize(GraphicsContext& graphicContext, Texture::Delegate& delegate) override {
        const bitmap bitmap = _bitmap_loader->get(_name);
        DCHECK(bitmap, "Texture resource \"%s\" not found", _name.c_str());
        delegate.uploadBitmap(graphicContext, bitmap, {bitmap->byteArray()});
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

const sp<Texture>& TextureBundle::createTexture(const String& src, const sp<Texture::Parameters>& parameters)
{
    sp<Texture>& texture = _textures[src];
    DCHECK_WARN(!texture, "Overriding Texture \"%s\"", src.c_str());
    texture = doCreateTexture(src, parameters);
    return texture;
}

sp<Texture> TextureBundle::doCreateTexture(const String& src, const sp<Texture::Parameters>& parameters) const
{
    const bitmap bitmapBounds = _bitmap_bounds_loader->get(src);
    DCHECK(bitmapBounds, "Texture resource \"%s\" not found", src.c_str());
    const sp<Size> size = sp<Size>::make(static_cast<float>(bitmapBounds->width()), static_cast<float>(bitmapBounds->height()));
    return _render_controller->createTexture(size, parameters, sp<UploaderBitmapBundle>::make(_bitmap_loader, src));
}

}
