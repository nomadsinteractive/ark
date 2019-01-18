#include "core/base/manifest.h"

#include "core/inf/asset.h"
#include "core/util/documents.h"

#include "platform/platform.h"

namespace ark {

Manifest::Manifest()
{
}

void Manifest::load(const String& src)
{
    Strings::rcut(Platform::getExecutablePath(), _application._dir, _application._filename, Platform::dirSeparator());

    const sp<ark::Asset> appAsset = Platform::getAsset(".", _application._dir);
    DASSERT(appAsset);
    const sp<Readable> readable = src ? appAsset->get(src) : nullptr;
    DCHECK(!src || readable, "Cannot load application manifest \"%s\"", src.c_str());
    const document manifest = readable ? Documents::loadFromReadable(readable) : document::make("");
    _asset._dir = Documents::getAttribute(manifest, "asset-dir");
}

const String& Manifest::name() const
{
    return _name;
}

const sp<Size>& Manifest::rendererResolution() const
{
    return _renderer._resolution;
}

const Manifest::Renderer& Manifest::renderer() const
{
    return _renderer;
}

}
