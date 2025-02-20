#pragma once

#include <map>

#include "core/ark.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/viewport.h"
#include "graphics/base/v2.h"

#include "renderer/forwarding.h"

#include "app/base/application_manifest.h"

namespace ark {

class ARK_API RenderEngineContext {
public:
    struct Resolution {
        uint32_t width;
        uint32_t height;
    };

public:
    RenderEngineContext(const ApplicationManifest::Renderer& renderer, const Viewport& viewport);

    const ApplicationManifest::Renderer& renderer() const;

    Enum::RendererVersion version() const;
    void setVersion(Enum::RendererVersion version);

    const Map<String, String>& definitions() const;
    Map<String, String>& definitions();

    const Viewport& viewport() const;
    void setViewport(const Viewport& viewport);

    const Resolution& displayResolution() const;
    void setDisplayResolution(const Resolution& displayResolution);

    const V2& displayUnit() const;

    void setSnippetFactory(sp<SnippetFactory> snippetfactory);
    const sp<SnippetFactory>& snippetFactory() const;

    uint32_t getGLSLVersion() const;

    Traits& traits();
    const Traits& traits() const;

private:
    ApplicationManifest::Renderer _renderer;
    Viewport _viewport;

    Map<String, String> _definitions;
    sp<SnippetFactory> _snippet_factory;
    Resolution _display_resolution;
    V2 _display_unit;

    Traits _traits;
};

}
