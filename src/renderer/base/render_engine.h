#ifndef ARK_RENDERER_INF_RENDER_ENGINE_H_
#define ARK_RENDERER_INF_RENDER_ENGINE_H_

#include <regex>

#include "core/ark.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class RenderEngine {
public:
    RenderEngine(Ark::RenderEngineVersion version, const sp<RenderViewFactory>& renderViewFactory);

    void initialize();

    Ark::RenderEngineVersion version() const;
    uint32_t getGLSLVersion() const;

    const String& inTypeName() const;
    const String& outTypeName() const;
    const String& fragmentName() const;

    const std::regex& inPattern() const;
    const std::regex& outPattern() const;

    sp<RenderView> createRenderView(const Viewport& viewport) const;

    sp<GLSnippet> createCoreGLSnippet(const sp<GLResourceManager>& glResourceManager, const sp<GLShader>& shader, const GLBuffer& arrayBuffer) const;

private:
    void chooseGLVersion(Ark::RenderEngineVersion version);

private:
    Ark::RenderEngineVersion _version;

    String _name;
    String _in_type_name;
    String _out_type_name;
    String _fragment_name;

    sp<RenderViewFactory> _render_view_factory;
    sp<GLProcedureFactory> _gl_procedure_factory;

    std::regex _in_pattern;
    std::regex _out_pattern;
};

}

#endif
