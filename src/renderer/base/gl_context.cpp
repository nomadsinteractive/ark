#include "renderer/base/gl_context.h"

#include "renderer/inf/gl_procedure_factory.h"

namespace ark {

GLContext::GLContext(Ark::GLVersion version)
    : _version(version)
{
}

Ark::GLVersion GLContext::version() const
{
    return _version;
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
        return static_cast<uint32_t>(_version) * 10;
    default:
        break;
    }
    return 110;
}

sp<GLSnippet> GLContext::createCoreGLSnippet(const sp<GLResourceManager>& glResourceManager, const sp<GLShader>& shader, const GLBuffer& arrayBuffer) const
{
    DCHECK(_gl_procedure_factory, "Uninitialized GLContext");
    return _gl_procedure_factory->createCoreGLSnippet(glResourceManager, shader, arrayBuffer);
}

}
