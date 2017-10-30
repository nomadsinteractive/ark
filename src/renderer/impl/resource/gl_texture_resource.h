#ifndef ARK_RENDERER_BASE_GL_TEXTURE_RESOURCE_H_
#define ARK_RENDERER_BASE_GL_TEXTURE_RESOURCE_H_

#include "core/inf/dictionary.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class GLTextureResource : public Dictionary<sp<GLTexture>> {
public:
    GLTextureResource(const sp<GLRecycler>& recycler, const sp<Dictionary<bitmap>>& bitmapLoader, const sp<Dictionary<bitmap>>& bitmapBoundsLoader);

    virtual sp<GLTexture> get(const String& name);

private:
    sp<GLRecycler> _recycler;

    sp<Dictionary<bitmap>> _bitmap_loader;
    sp<Dictionary<bitmap>> _bitmap_bounds_loader;
};

}

#endif
