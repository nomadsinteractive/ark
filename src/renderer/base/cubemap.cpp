#include "renderer/base/cubemap.h"


namespace ark {

Cubemap::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
{
}

sp<Texture> Cubemap::BUILDER::build(const sp<Scope>& args)
{
    return nullptr;
}

}
