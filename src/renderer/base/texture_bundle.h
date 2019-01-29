#ifndef ARK_RENDERER_BASE_TEXTURE_BUNDLE_H_
#define ARK_RENDERER_BASE_TEXTURE_BUNDLE_H_

#include <map>

#include "core/base/string.h"
#include "core/inf/dictionary.h"
#include "core/types/shared_ptr.h"

#include "renderer/base/texture.h"

namespace ark {

class TextureBundle : public Dictionary<sp<Texture>> {
public:
    TextureBundle(const sp<RenderController>& renderController);

    virtual sp<Texture> get(const String& name) override;

private:
    sp<RenderController> _render_controller;
    sp<Dictionary<sp<Texture>>> _delegate;

    std::map<String, sp<Texture>> _textures;
};

}

#endif
