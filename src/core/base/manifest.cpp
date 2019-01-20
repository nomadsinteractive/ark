#include "core/base/manifest.h"

#include "core/inf/asset.h"
#include "core/util/documents.h"

#include "graphics/base/size.h"

#include "platform/platform.h"

namespace ark {

Manifest::Manifest()
{
}

Manifest::Manifest(const String& src)
{
    load(src);
}

void Manifest::load(const String& src)
{
    Strings::rcut(Platform::getExecutablePath(), _application._dir, _application._filename, Platform::dirSeparator());

    const sp<ark::Asset> appAsset = Platform::getAsset(".", _application._dir);
    DASSERT(appAsset);
    const sp<Readable> readable = appAsset->get(src);
    DCHECK(readable, "Cannot load application manifest \"%s\"", src.c_str());

    _content = readable ? Documents::loadFromReadable(readable) : document::make("");
    _asset_dir = Documents::getAttribute(_content, "asset-dir");
    _assets = Documents::getKeyValuePairs<Table<String, String>>(_content->children("asset"), "prefix", Constants::Attributes::SRC);

    _plugins = Documents::getList<std::vector<String>>(_content->children("plugin"), Constants::Attributes::NAME);

    const document& renderer = _content->getChild("renderer");
    if(renderer)
    {
        const document& resolution = renderer->getChild("resolution");
        if(resolution)
            _renderer._resolution = sp<Size>::make(Documents::ensureAttribute<float>(resolution, Constants::Attributes::WIDTH),
                                                   Documents::ensureAttribute<float>(resolution, Constants::Attributes::HEIGHT));
        _renderer._version = Documents::getAttribute<Ark::RendererVersion>(renderer, "version", Ark::AUTO);
    }
}

const String& Manifest::name() const
{
    return _name;
}

const String& Manifest::appDir() const
{
    return _application._dir;
}

const String& Manifest::assetDir() const
{
    return _asset_dir;
}

const Table<String, String>& Manifest::assets() const
{
    return _assets;
}

const std::vector<String>& Manifest::plugins() const
{
    return _plugins;
}

const sp<Size>& Manifest::rendererResolution() const
{
    return _renderer._resolution;
}

const Manifest::Renderer& Manifest::renderer() const
{
    return _renderer;
}

const document& Manifest::content() const
{
    return _content;
}

Manifest::Renderer::Renderer()
    : _version(Ark::AUTO)
{
}

}
