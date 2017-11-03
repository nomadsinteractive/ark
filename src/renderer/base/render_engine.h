#ifndef ARK_RENDERER_INF_RENDER_ENGINE_H_
#define ARK_RENDERER_INF_RENDER_ENGINE_H_

#include "core/ark.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class RenderEngine {
public:
    RenderEngine(Ark::GLVersion version, const sp<RenderViewFactory>& renderViewFactory);

    void initialize();

    sp<RenderView> createRenderView(const Viewport& viewport) const;
    sp<GLSnippet> createCoreGLSnippet(const sp<GLResourceManager>& glResourceManager, const sp<GLShader>& shader, const GLBuffer& arrayBuffer) const;

private:
    void chooseGLVersion(Ark::GLVersion version);

private:
    sp<GLContext> _gl_context;

    sp<RenderViewFactory> _render_view_factory;
    sp<GLProcedureFactory> _gl_procedure_factory;

};

}

#endif
