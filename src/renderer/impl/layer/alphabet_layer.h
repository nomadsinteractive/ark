#ifndef ARK_RENDERER_IMPL_LAYER_ALPHABET_LAYER_H_
#define ARK_RENDERER_IMPL_LAYER_ALPHABET_LAYER_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class AlphabetLayer {
public:

//  [[plugin::resource-loader("text-layer")]]
    class BUILDER : public Builder<Layer> {
    public:
        BUILDER(BeanFactory& parent, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<Layer> build(const sp<Scope>& args);

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        sp<Builder<Alphabet>> _alphabet;
        sp<Builder<GLShader>> _shader;
        sp<Builder<Vec4>> _color;

        uint32_t _texture_width, _texture_height;
    };
};

}

#endif
