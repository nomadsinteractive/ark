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

namespace ark {

class ARK_API Manifest {
public:
    struct Application {
        String _dir;
        String _filename;
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

        sp<Size> _resolution;
        Ark::RendererVersion _version;
    };

public:
    Manifest();
    Manifest(const String& src);

    void load(const String& src);

// [[script::bindings::property]]
    const String& name() const;

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
