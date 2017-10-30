#ifndef ARK_PLUGIN_NOISE_IMPL_RESOURCE_NOISE_TEXTURE_RESOURCE_H_
#define ARK_PLUGIN_NOISE_IMPL_RESOURCE_NOISE_TEXTURE_RESOURCE_H_

#include "core/inf/builder.h"
#include "core/inf/dictionary.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class NoiseTextureResource : public Dictionary<sp<GLTexture>> {
public:
    virtual sp<GLTexture> get(const String& name) override;


private:
    sp<GLResourceManager> _resource_manager;
    sp<Executor> _thread_executor;
};

}

#endif
