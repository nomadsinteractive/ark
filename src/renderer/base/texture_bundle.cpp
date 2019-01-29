#include "renderer/base/texture_bundle.h"

#include "renderer/base/render_controller.h"

namespace ark {

TextureBundle::TextureBundle(const sp<RenderController>& renderController)
    : _render_controller(renderController), _delegate(_render_controller->createTextureBundle())
{
}

sp<Texture> TextureBundle::get(const String& name)
{
    auto iter = _textures.find(name);
    if(iter != _textures.end())
        return iter->second;

    const sp<Texture> texture = _delegate->get(name);
    DCHECK(texture, "Texture \"%s\" not loaded", name.c_str());
    _render_controller->upload(texture, nullptr, RenderController::US_ONCE_AND_ON_SURFACE_READY);

    _textures[name] = texture;
    return texture;
}

}
