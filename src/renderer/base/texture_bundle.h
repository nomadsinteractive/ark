#ifndef ARK_RENDERER_BASE_TEXTURE_BUNDLE_H_
#define ARK_RENDERER_BASE_TEXTURE_BUNDLE_H_

#include <map>

#include "core/base/string.h"
#include "core/inf/dictionary.h"
#include "core/types/shared_ptr.h"

#include "renderer/base/texture.h"

#include "renderer/base/texture.h"

namespace ark {

class TextureBundle {
public:
    TextureBundle(const sp<RenderController>& renderController);

    const sp<Texture>& getTexture(const String& src);
    const sp<Texture>& createTexture(const String& src, const sp<Texture::Parameters>& parameters);

private:
    sp<Texture> doCreateTexture(const String& src, const sp<Texture::Parameters>& parameters) const;

private:
    sp<RenderController> _render_controller;
    sp<RendererFactory> _renderer_factory;
    sp<Dictionary<bitmap>> _bitmap_loader;
    sp<Dictionary<bitmap>> _bitmap_bounds_loader;

    std::map<String, sp<Texture>> _textures;
};

}

#endif
