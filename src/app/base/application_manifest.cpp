#include "app/base/application_manifest.h"

#include "core/inf/asset.h"
#include "core/inf/asset_bundle.h"
#include "core/util/string_convert.h"
#include "core/util/documents.h"

#include "graphics/base/size.h"

#include "platform/platform.h"

namespace ark {

ApplicationManifest::ApplicationManifest()
{
}

ApplicationManifest::ApplicationManifest(const String& src)
{
    load(src);
}

void ApplicationManifest::load(const String& src)
{
    auto [appDirOpt, appFileName] = Platform::getExecutablePath().rcut(Platform::dirSeparator());
    if(appDirOpt)
        _application._dir = std::move(appDirOpt.value());
    _application._filename = std::move(appFileName);

    const sp<ark::AssetBundle> appAsset = Platform::getAssetBundle(".", _application._dir);
    DASSERT(appAsset);
    const sp<ark::Asset> asset = appAsset->getAsset(src);
    DCHECK(asset, "Cannot load application manifest \"%s\"", src.c_str());

    _content = asset ? Documents::loadFromReadable(asset->open()) : document::make("");
    _asset_dir = Documents::getAttribute(_content, "asset-dir");
    for(const document& i : _content->children("asset"))
        _assets.push_back(Asset(i));

    _plugins = Documents::getSystemSpecificList<std::vector<String>>(_content->children("plugin"), constants::NAME);

    const document& renderer = _content->getChild("renderer");
    if(renderer)
    {
        const document& resolution = renderer->getChild("resolution");
        if(resolution)
            _renderer._resolution = sp<Size>::make(Documents::ensureAttribute<float>(resolution, constants::WIDTH),
                                                   Documents::ensureAttribute<float>(resolution, constants::HEIGHT));
        _renderer._version = Documents::getAttribute<Ark::RendererVersion>(renderer, "version", Ark::RENDERER_VERSION_AUTO);
        _renderer._coordinate_system = Documents::getAttribute<Ark::RendererCoordinateSystem>(renderer, "coordinate-system", Ark::COORDINATE_SYSTEM_DEFAULT);
        _renderer._vsync = Documents::getAttribute<bool>(renderer, "vsync", false);
    }

    _heap._device_unit_size = toSize(Documents::getAttributeValue(_content, "heap/device/unit-size", "8M"));
    _heap._host_unit_size = toSize(Documents::getAttributeValue(_content, "heap/host/unit-size", "8M"));

    _application._title = Documents::getAttributeValue(_content, "application/title");
    _application._window_flag = Documents::getAttributeValue<WindowFlag>(_content, "application/window-flag", WINDOW_FLAG_SHOW_CURSOR);

    const String messageLoopType = Documents::getAttributeValue(_content, "application/message-loop", "core");
    if(messageLoopType == "core")
        _application._message_loop = MESSAGE_LOOP_TYPE_CORE;
    else if(messageLoopType == "render")
        _application._message_loop = MESSAGE_LOOP_TYPE_RENDER;
    else
        DFATAL("Unknow application message-loop: \"%s\". Should be \"core\" or \"render\"", messageLoopType.c_str());

    _resource_loader = _content->getChild("resource-loader");
    if(!_resource_loader)
        _resource_loader = sp<DOMDocument>::make("resource-loader");
}

const String& ApplicationManifest::name() const
{
    return _application._title;
}

const ApplicationManifest::Application& ApplicationManifest::application() const
{
    return _application;
}

const String& ApplicationManifest::appDir() const
{
    return _application._dir;
}

const String& ApplicationManifest::assetDir() const
{
    return _asset_dir;
}

const std::vector<ApplicationManifest::Asset>& ApplicationManifest::assets() const
{
    return _assets;
}

const std::vector<String>& ApplicationManifest::plugins() const
{
    return _plugins;
}

const sp<Size>& ApplicationManifest::rendererResolution() const
{
    DCHECK(_renderer._resolution, "RenderResolution undefined in manifest");
    return _renderer._resolution;
}

const ApplicationManifest::Heap& ApplicationManifest::heap() const
{
    return _heap;
}

const ApplicationManifest::Renderer& ApplicationManifest::renderer() const
{
    return _renderer;
}

const document& ApplicationManifest::content() const
{
    return _content;
}

const document& ApplicationManifest::resourceLoader() const
{
    return _resource_loader;
}

uint32_t ApplicationManifest::toSize(const String& sizestr) const
{
    const String s = sizestr.toLower();
    const std::pair<String, uint32_t> suffixs[] = {{"k", 10}, {"kb", 10}, {"m", 20}, {"mb", 20}, {"g", 30}, {"gb", 30}};
    for(const std::pair<String, uint32_t>& i : suffixs)
        if(s.endsWith(i.first))
            return Strings::eval<uint32_t>(s.substr(0, s.length() - i.first.length())) << i.second;
    return Strings::eval<uint32_t>(s);
}

ApplicationManifest::Renderer::Renderer()
    : _version(Ark::RENDERER_VERSION_AUTO)
{
}

Viewport ApplicationManifest::Renderer::toViewport() const
{
    DASSERT(_resolution);
    return Viewport(0, 0, _resolution->widthAsFloat(), _resolution->heightAsFloat(), -1.0f, 1.0f);
}

static ApplicationManifest::WindowFlag toOneWindowFlag(const String& val)
{
    const String s = val.toLower();
    if(s == "show_cursor")
        return ApplicationManifest::WINDOW_FLAG_SHOW_CURSOR;
    if(s == "resizable")
        return ApplicationManifest::WINDOW_FLAG_RESIZABLE;
    if(s == "maxinized")
        return ApplicationManifest::WINDOW_FLAG_MAXINIZED;
    if(s == "full_screen")
        return ApplicationManifest::WINDOW_FLAG_FULL_SCREEN;
    if(s == "full_screen_windowed")
        return ApplicationManifest::WINDOW_FLAG_FULL_SCREEN_WINDOWED;
    DFATAL("Unknow window flag: %s", val.c_str());
    return ApplicationManifest::WINDOW_FLAG_NONE;
}

template<> ARK_API ApplicationManifest::WindowFlag StringConvert::eval<ApplicationManifest::WindowFlag>(const String& val)
{
    uint32_t v = ApplicationManifest::WINDOW_FLAG_NONE;
    for(const String& i : val.split('|'))
        v |= toOneWindowFlag(i);
    return static_cast<ApplicationManifest::WindowFlag>(v);
}

ApplicationManifest::Asset::Asset(const document& manifest)
    : _protocol(Documents::getAttribute(manifest, "protocol")), _root(Documents::getAttribute(manifest, "root", "/")),
      _src(Documents::getAttribute(manifest, constants::SRC))
{
}

}
