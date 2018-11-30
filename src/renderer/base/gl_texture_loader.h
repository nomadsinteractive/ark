#ifndef ARK_RENDERER_BASE_TEXTURE_LOADER_H_
#define ARK_RENDERER_BASE_TEXTURE_LOADER_H_

#include <map>

#include "core/types/shared_ptr.h"
#include "core/inf/dictionary.h"

#include "renderer/base/texture.h"

namespace ark {

class GLTextureLoader : public Dictionary<sp<Texture>> {
public:
    GLTextureLoader(const sp<GLResourceManager>& resources);

    virtual sp<Texture> get(const String& name) override;

private:
    sp<GLResourceManager> _resource_manager;

    std::map<String, sp<Texture>> _textures;
};

}

#endif
