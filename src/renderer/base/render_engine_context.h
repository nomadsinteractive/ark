#pragma once

#include <map>

#include "core/ark.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/viewport.h"
#include "graphics/base/v2.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API RenderEngineContext {
public:
    struct Resolution {
        uint32_t width;
        uint32_t height;
    };

public:
    RenderEngineContext(Ark::RendererVersion version, const Viewport& viewport);

    Ark::RendererVersion version() const;
    void setVersion(Ark::RendererVersion version);

    const std::map<String, String>& definitions() const;
    std::map<String, String>& definitions();

    const Viewport& viewport() const;
    void setViewport(const Viewport& viewport);

    const Resolution& displayResolution() const;
    void setDisplayResolution(const Resolution& displayResolution);

    const V2& displayUnit() const;

    void setSnippetFactory(sp<SnippetFactory> snippetfactory);
    const sp<SnippetFactory>& snippetFactory() const;

    uint32_t getGLSLVersion() const;

private:
    Ark::RendererTarget _target;
    Ark::RendererVersion _version;
    Viewport _viewport;

    std::map<String, String> _annotations;
    sp<SnippetFactory> _snippet_factory;
    Resolution _display_resolution;
    V2 _display_unit;
};

}
