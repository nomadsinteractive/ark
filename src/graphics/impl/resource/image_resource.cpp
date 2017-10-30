#include "graphics/impl/resource/image_resource.h"

#include "core/util/strings.h"

namespace ark {

ImageResource::ImageResource(const sp<Dictionary<bitmap>>& defaultLoader)
    : _default_loader(defaultLoader)
{
}

bitmap ImageResource::get(const String& name)
{
    String fname, fext;
    Strings::rcut(name, fname, fext, '.');

    auto iter = _loaders.find(fext);
    if(iter != _loaders.end())
        return iter->second->get(name);
    DCHECK(_default_loader, "Counld not find loader for image extension \"%s\"", fext.c_str());
    return _default_loader->get(name);
}

void ImageResource::addLoader(const String& ext, const sp<Dictionary<bitmap>>& loader)
{
    _loaders[ext] = loader;
}

}
