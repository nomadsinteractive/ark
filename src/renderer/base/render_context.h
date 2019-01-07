#ifndef ARK_RENDERER_BASE_RENDER_CONTEXT_H_
#define ARK_RENDERER_BASE_RENDER_CONTEXT_H_

#include <map>

#include "core/ark.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/viewport.h"

#include "renderer/forwarding.h"

namespace ark {

class RenderContext {
public:
    RenderContext(Ark::RendererVersion version, const Viewport& viewport);

    Ark::RendererVersion version() const;
    void setVersion(Ark::RendererVersion version);

    const std::map<String, String>& annotations() const;
    std::map<String, String>& annotations();

    const Viewport& viewport() const;

    void setSnippetFactory(sp<SnippetFactory> snippetfactory);
    const sp<SnippetFactory>& snippetFactory() const;

    uint32_t getGLSLVersion() const;

private:
    Ark::RendererVersion _version;
    std::map<String, String> _annotations;

    Viewport _viewport;
    sp<SnippetFactory> _snippet_factory;
};

}

#endif
