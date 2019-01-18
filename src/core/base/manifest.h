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
        Table<String, String> _entries;
    };

    struct Renderer {
        sp<Size> _resolution;
        Ark::RendererVersion _version;
    };

public:
    Manifest();

    void load(const String& src);

// [[script::bindings::property]]
    const String& name() const;

// [[script::bindings::property]]
    const sp<Size>& rendererResolution() const;

    const Renderer& renderer() const;

private:
    String _name;

    Application _application;
    Asset _asset;
    Renderer _renderer;

    std::vector<String> _plugins;
};

}

#endif
