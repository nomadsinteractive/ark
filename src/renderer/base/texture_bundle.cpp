#include "renderer/base/texture_bundle.h"

#include "renderer/base/render_controller.h"
#include "renderer/base/resource_manager.h"

namespace ark {

TextureBundle::TextureBundle(RenderController& renderController)
    : _resource_manager(renderController.resourceManager()), _delegate(renderController.createTextureBundle())
{
}

sp<Texture> TextureBundle::get(const String& name)
{
    auto iter = _textures.find(name);
    if(iter != _textures.end())
        return iter->second;

    const sp<Texture> texture = _delegate->get(name);
    DCHECK(texture, "Texture \"%s\" not loaded", name.c_str());
    _resource_manager->upload(texture, nullptr, ResourceManager::US_ONCE_AND_ON_SURFACE_READY);

    _textures[name] = texture;
    return texture;
}

}
