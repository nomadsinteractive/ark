#ifndef ARK_GRAPHICS_IMPL_RESOURCE_IMAGE_RESOURCE_H_
#define ARK_GRAPHICS_IMPL_RESOURCE_IMAGE_RESOURCE_H_

#include <map>

#include "core/inf/dictionary.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ImageResource : public Dictionary<bitmap> {
public:
    ImageResource(const sp<Dictionary<bitmap>>& defaultLoader);

    virtual bitmap get(const String& name) override;

    void addLoader(const String& ext, const sp<Dictionary<bitmap>>& loader);

private:
    std::map<String, sp<Dictionary<bitmap>>> _loaders;
    sp<Dictionary<bitmap>> _default_loader;

};

}

#endif
