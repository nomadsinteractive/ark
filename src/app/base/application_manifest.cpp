#include "app/base/application_manifest.h"

#include "core/inf/asset.h"
#include "core/inf/asset_bundle.h"
#include "core/util/string_convert.h"
#include "core/util/documents.h"

#include "graphics/components/size.h"

#include "platform/platform.h"

namespace ark {

namespace {

uint32_t toSize(const String& sizestr)
{
    const String s = sizestr.toLower();
    const std::pair<String, uint32_t> suffixs[] = {{"k", 10}, {"kb", 10}, {"m", 20}, {"mb", 20}, {"g", 30}, {"gb", 30}};
    for(const auto& [k, v] : suffixs)
        if(s.endsWith(k))
            return Strings::eval<uint32_t>(s.substr(0, s.length() - k.length())) << v;
    return Strings::eval<uint32_t>(s);
}

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

    const sp<AssetBundle> appAsset = Platform::getAssetBundle(".", _application._dir);
    DASSERT(appAsset);
    const sp<ark::Asset> asset = appAsset->getAsset(src);
    DCHECK(asset, "Cannot load application manifest \"%s\"", src.c_str());

    _content = asset ? Documents::loadFromReadable(asset->open()) : document::make("");
    _asset_dir = Documents::getAttribute(_content, "asset-dir");
    for(const document& i : _content->children("asset"))
        _assets.emplace_back(i);

    _plugins = Documents::getSystemSpecificList<Vector<String>>(_content->children("plugin"), constants::NAME);

    if(const document& renderer = _content->getChild("renderer"))
        _renderer = Renderer(renderer);

    _heap._device_unit_size = toSize(Documents::getAttributeValue(_content, "heap/device/unit-size", "8M"));
    _heap._host_unit_size = toSize(Documents::getAttributeValue(_content, "heap/host/unit-size", "8M"));

    _interpreter = _content->getChild("interpreter");

    _application._title = Documents::getAttributeValue(_content, "application/title");

    _window._title = Documents::getAttributeValue(_content, "window/title", _application._title);
    _window._flags = Documents::getAttributeValue<WindowFlags>(_content, "window/flags", WINDOW_FLAG_SHOW_CURSOR);
    _window._position_x = Documents::getAttributeValue<int32_t>(_content, "window/x", WINDOW_POSITION_CENTERED);
    _window._position_y = Documents::getAttributeValue<int32_t>(_content, "window/y", WINDOW_POSITION_CENTERED);
    _window._scale = Documents::getAttributeValue<float>(_content, "window/scale", 1.0f);

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

const ApplicationManifest::Window& ApplicationManifest::window() const
{
    return _window;
}

const String& ApplicationManifest::assetDir() const
{
    return _asset_dir;
}

const Vector<ApplicationManifest::Asset>& ApplicationManifest::assets() const
{
    return _assets;
}

const Vector<String>& ApplicationManifest::plugins() const
{
    return _plugins;
}

const V2& ApplicationManifest::rendererResolution() const
{
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

const document& ApplicationManifest::interpreter() const
{
    return _interpreter;
}

ApplicationManifest::Renderer::Renderer()
    : _backend(Enum::RENDERING_BACKEND_AUTO), _version(Enum::RENDERER_VERSION_AUTO), _coordinate_system(Ark::COORDINATE_SYSTEM_DEFAULT), _vsync(false), _resolution(1920, 1080)
{
}

ApplicationManifest::Renderer::Renderer(const document& manifest)
    : _class(Documents::getAttribute(manifest, constants::CLASS)), _backend(Documents::getAttribute(manifest, "backend", Enum::RENDERING_BACKEND_AUTO)), _version(Documents::getAttribute(manifest, "version", Enum::RENDERER_VERSION_AUTO)),
      _coordinate_system(Documents::getAttribute(manifest, "coordinate-system", Ark::COORDINATE_SYSTEM_DEFAULT)), _vsync(Documents::getAttribute(manifest, "vsync", false))
{
    if(const document& resolution = manifest->getChild("resolution"))
        _resolution = {Documents::ensureAttribute<float>(resolution, constants::WIDTH), Documents::ensureAttribute<float>(resolution, constants::HEIGHT)};
    else
        _resolution = {1920, 1080};
}

template<> ARK_API ApplicationManifest::WindowFlags StringConvert::eval<ApplicationManifest::WindowFlags>(const String& val)
{
    constexpr ApplicationManifest::WindowFlags::LookupTable<6> windowFlags = {{
        {"none", ApplicationManifest::WINDOW_FLAG_NONE},
        {"show_cursor", ApplicationManifest::WINDOW_FLAG_SHOW_CURSOR},
        {"resizable", ApplicationManifest::WINDOW_FLAG_RESIZABLE},
        {"maxinized", ApplicationManifest::WINDOW_FLAG_MAXINIZED},
        {"full_screen", ApplicationManifest::WINDOW_FLAG_FULL_SCREEN},
        {"borderless", ApplicationManifest::WINDOW_FLAG_FULL_SCREEN_WINDOWED},
    }};
    return ApplicationManifest::WindowFlags::toBitSet(val, windowFlags);
}

ApplicationManifest::Asset::Asset(const document& manifest)
    : _protocol(Documents::getAttribute(manifest, "protocol")), _root(Documents::getAttribute(manifest, "root", "/")),
      _src(Documents::getAttribute(manifest, constants::SRC))
{
}

}
