#include "renderer/base/gl_context.h"

#include "renderer/inf/snippet_factory.h"

namespace ark {

GLContext::GLContext(Ark::RendererVersion version)
    : _version(version)
{
}

Ark::RendererVersion GLContext::version() const
{
    return _version;
}

void GLContext::setVersion(Ark::RendererVersion version)
{
    _version = version;
}

const std::map<String, String>& GLContext::annotations() const
{
    return _annotations;
}

std::map<String, String>& GLContext::annotations()
{
    return _annotations;
}

uint32_t GLContext::getGLSLVersion() const
{
    switch(_version) {
    case Ark::OPENGL_20:
        return 110;
    case Ark::OPENGL_21:
        return 120;
    case Ark::OPENGL_30:
        return 130;
    case Ark::OPENGL_31:
        return 140;
    case Ark::OPENGL_32:
        return 150;
    case Ark::OPENGL_33:
    case Ark::OPENGL_40:
    case Ark::OPENGL_41:
    case Ark::OPENGL_42:
    case Ark::OPENGL_43:
    case Ark::OPENGL_44:
    case Ark::OPENGL_45:
    case Ark::OPENGL_46:
        return static_cast<uint32_t>(_version) * 10;
    default:
        break;
    }
    FATAL("Unsupported OpenGL version: %d", _version);
    return 110;
}

void GLContext::setGLSnippetFactory(sp<SnippetFactory> snippetfactory)
{
    _gl_procedure_factory = std::move(snippetfactory);
}

sp<Snippet> GLContext::createCoreGLSnippet(const sp<ResourceManager>& glResourceManager, const sp<ShaderBindings>& shaderBindings) const
{
    DCHECK(_gl_procedure_factory, "Uninitialized GLContext");
    return _gl_procedure_factory->createCoreSnippet(glResourceManager, shaderBindings);
}

}
