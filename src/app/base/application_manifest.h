#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/bit_set.h"
#include "core/base/string.h"
#include "core/base/url.h"
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

    enum WindowFlagBits {
        WINDOW_FLAG_NONE = 0,
        WINDOW_FLAG_SHOW_CURSOR = 1,
        WINDOW_FLAG_RESIZABLE = 2,
        WINDOW_FLAG_MAXINIZED = 4,
        WINDOW_FLAG_FULL_SCREEN = 8,
        WINDOW_FLAG_FULL_SCREEN_WINDOWED = 16
    };
    typedef BitSet<WindowFlagBits> WindowFlags;

    enum WindowPosition {
        WINDOW_POSITION_UNDEFINED = -2,
        WINDOW_POSITION_CENTERED = -1
    };

    struct Application {
        String _dir;
        String _filename;
        String _title;
        MessageLoopType _message_loop;
    };

    struct Window {
        String _title;
        WindowFlags _flags;
        int32_t _position_x;
        int32_t _position_y;
        float _scale;
    };

    struct Asset {
        String _root;
        URL _src;

        Asset(const document& manifest);
        DEFAULT_COPY_AND_ASSIGN(Asset);
    };

    struct Heap {
        uint32_t _host_unit_size;
        uint32_t _device_unit_size;
    };

    struct Renderer {
        Renderer();
        Renderer(const document& manifest);

        String _class;

        enums::RenderingBackendBit _backend;
        enums::RendererVersion _version;
        enums::CoordinateSystem _coordinate_system;

        bool _vsync;
        V2 _resolution;
    };

public:
    ApplicationManifest() = default;
    ApplicationManifest(const String& src);

    void load(const String& src);

// [[script::bindings::property]]
    const String& name() const;

    const Application& application() const;
    const Window& window() const;

    const Vector<Asset>& assets() const;
    const Vector<String>& plugins() const;

// [[script::bindings::property]]
    const V2& rendererResolution() const;

    const Heap& heap() const;
    const Renderer& renderer() const;

    const document& content() const;
    const document& resourceLoader() const;
    const document& interpreter() const;

private:
    String _name;

    Vector<Asset> _assets;
    Vector<String> _plugins;

    Application _application;
    Window _window;
    Heap _heap;
    Renderer _renderer;

    document _content;
    document _resource_loader;
    document _interpreter;
};

}
