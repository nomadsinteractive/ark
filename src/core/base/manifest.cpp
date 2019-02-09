#include "core/base/manifest.h"

#include "core/inf/asset.h"
#include "core/inf/asset_bundle.h"
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

    const sp<ark::AssetBundle> appAsset = Platform::getAsset(".", _application._dir);
    DASSERT(appAsset);
    const sp<ark::Asset> asset = appAsset->get(src);
    DCHECK(asset, "Cannot load application manifest \"%s\"", src.c_str());

    _content = asset ? Documents::loadFromReadable(asset->open()) : document::make("");
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

    _heap._device_unit_size = toSize(Documents::getAttributeValue(_content, "heap/device/unit-size", "8M"));
    _heap._host_unit_size = toSize(Documents::getAttributeValue(_content, "heap/host/unit-size", "8M"));

    _application._title = Documents::getAttributeValue(_content, "application/title");
}

const String& Manifest::name() const
{
    return _application._title;
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

const Manifest::Heap& Manifest::heap() const
{
    return _heap;
}

const Manifest::Renderer& Manifest::renderer() const
{
    return _renderer;
}

const document& Manifest::content() const
{
    return _content;
}

uint32_t Manifest::toSize(const String& sizestr) const
{
    const String s = sizestr.toLower();
    const std::pair<String, uint32_t> suffixs[] = {{"k", 10}, {"kb", 10}, {"m", 20}, {"mb", 20}, {"g", 30}, {"gb", 30}};
    for(const std::pair<String, uint32_t>& i : suffixs)
        if(s.endsWith(i.first))
            return Strings::parse<uint32_t>(s.substr(0, s.length() - i.first.length())) << i.second;
    return Strings::parse<uint32_t>(s);
}

Manifest::Renderer::Renderer()
    : _version(Ark::AUTO)
{
}

}
