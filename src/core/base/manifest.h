#ifndef ARK_CORE_BASE_MANIFEST_H_
#define ARK_CORE_BASE_MANIFEST_H_

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

class ARK_API Manifest {
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
        String _dir;

    };

    struct Heap {
        uint32_t _host_unit_size;
        uint32_t _device_unit_size;
    };

    struct Renderer {
        Renderer();

        Ark::RendererVersion _version;
        sp<Size> _resolution;

        Viewport toViewport() const;
    };

public:
    Manifest();
    Manifest(const String& src);

    void load(const String& src);

// [[script::bindings::property]]
    const String& name() const;

    const Application& application() const;

    const String& appDir() const;
    const String& assetDir() const;

    const Table<String, String>& assets() const;
    const std::vector<String>& plugins() const;

// [[script::bindings::property]]
    const sp<Size>& rendererResolution() const;

    const Heap& heap() const;
    const Renderer& renderer() const;

    const document& content() const;

private:
    uint32_t toSize(const String& sizestr) const;

private:
    String _name;

    String _asset_dir;

    Table<String, String> _assets;
    std::vector<String> _plugins;

    Application _application;
    Heap _heap;
    Renderer _renderer;

    document _content;

};

}

#endif
