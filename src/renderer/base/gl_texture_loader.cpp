#include "renderer/base/gl_texture_loader.h"

#include "renderer/base/gl_resource_manager.h"

namespace ark {

GLTextureLoader::GLTextureLoader(const sp<GLResourceManager>& resources)
    : _resource_manager(resources)
{
}

sp<GLTexture> GLTextureLoader::get(const String& name)
{
    auto iter = _textures.find(name);
    if(iter != _textures.end())
        return iter->second;

    const sp<GLTexture> texture = _resource_manager->loadGLTexture(name);
    _textures[name] = texture;
    return _textures[name];
}

}
