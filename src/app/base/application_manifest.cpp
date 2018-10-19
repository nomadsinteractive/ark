#include "app/base/application_manifest.h"

#include "core/util/documents.h"

#include "graphics/base/size.h"

namespace ark {

ApplicationManifest::ApplicationManifest(const document& manifest)
    : _name(Documents::ensureAttribute(manifest, Constants::Attributes::NAME)), _manifest(manifest)
{
    const document& s = manifest->getChild("render-resolution");
    DASSERT(s);
    _render_resolution = sp<Size>::make(Documents::ensureAttribute<float>(s, Constants::Attributes::WIDTH), Documents::ensureAttribute<float>(s, Constants::Attributes::HEIGHT));
}

const String& ApplicationManifest::name() const
{
    return _name;
}

const sp<Size>& ApplicationManifest::renderResolution() const
{
    return _render_resolution;
}

const document& ApplicationManifest::manifest() const
{
    return _manifest;
}


}
