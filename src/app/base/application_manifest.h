#pragma once

#include <vector>

#include "core/ark.h"
#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/collection/table.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/viewport.h"

namespace ark {

class ARK_API ApplicationManifest {
public:
    enum MessageLoopType {
        MESSAGE_LOOP_TYPE_CORE,
        MESSAGE_LOOP_TYPE_RENDER
    };

    enum WindowFlag {
        WINDOW_FLAG_NONE = 0,
        WINDOW_FLAG_SHOW_CURSOR = 1,
        WINDOW_FLAG_RESIZABLE = 2,
        WINDOW_FLAG_MAXINIZED = 4,
        WINDOW_FLAG_FULL_SCREEN = 8,
        WINDOW_FLAG_FULL_SCREEN_WINDOWED = 16
    };

    struct Application {
        String _dir;
        String _filename;
        String _title;
        MessageLoopType _message_loop;
        WindowFlag _window_flag;
    };

    struct Asset {
        String _protocol;
        String _root;
        String _src;

        Asset(const document& manifest);
        DEFAULT_COPY_AND_ASSIGN(Asset);
    };

    struct Heap {
        uint32_t _host_unit_size;
        uint32_t _device_unit_size;
    };

    struct Renderer {
        Renderer();

        Ark::RendererVersion _version;
        Ark::RendererCoordinateSystem _coordinate_system;

        bool _vsync;
        sp<Size> _resolution;

        Viewport toViewport() const;
    };

public:
    ApplicationManifest();
    ApplicationManifest(const String& src);

    void load(const String& src);

// [[script::bindings::property]]
    const String& name() const;

    const Application& application() const;

    const String& appDir() const;
    const String& assetDir() const;

    const std::vector<Asset>& assets() const;
    const std::vector<String>& plugins() const;

// [[script::bindings::property]]
    const sp<Size>& rendererResolution() const;

    const Heap& heap() const;
    const Renderer& renderer() const;

    const document& content() const;
    const document& resourceLoader() const;
    const document& interpreter() const;

private:
    uint32_t toSize(const String& sizestr) const;

private:
    String _name;

    String _asset_dir;

    std::vector<Asset> _assets;
    std::vector<String> _plugins;

    Application _application;
    Heap _heap;
    Renderer _renderer;

    document _content;
    document _resource_loader;
    document _interpreter;
};

}
