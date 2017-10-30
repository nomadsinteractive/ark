#ifndef ARK_RENDERER_BASE_TEXTURE_LOADER_H_
#define ARK_RENDERER_BASE_TEXTURE_LOADER_H_

#include <map>

#include "core/types/shared_ptr.h"
#include "core/inf/dictionary.h"

#include "renderer/base/gl_texture.h"

namespace ark {

class GLTextureLoader : public Dictionary<sp<GLTexture>> {
public:
    GLTextureLoader(const sp<GLResourceManager>& resources);

    virtual sp<GLTexture> get(const String& name) override;

private:
    sp<GLResourceManager> _resource_manager;

    std::map<String, sp<GLTexture>> _textures;
};

}

#endif
