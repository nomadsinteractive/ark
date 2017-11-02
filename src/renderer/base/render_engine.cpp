#include "renderer/base/render_engine.h"

#include "core/base/string_table.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "renderer/inf/render_view_factory.h"
#include "renderer/inf/gl_procedure_factory.h"

#include "renderer/gles20/impl/gl_procedure_factory/gl_procedure_factory_gles20.h"
#include "renderer/gles30/impl/gl_procedure_factory/gl_procedure_factory_gles30.h"

#include "platform/gl/gl.h"

namespace ark {

static const char _var_pattern[] = "%s\\s+([\\w\\d]+)\\s+(?:a_|v_)([\\w\\d_]+);";

RenderEngine::RenderEngine(Ark::RenderEngineVersion version, const sp<RenderViewFactory>& renderViewFactory)
    : _version(version), _in_type_name("${vert.in}"), _out_type_name("${vert.out}"), _fragment_name("${frag.color}"), _render_view_factory(renderViewFactory),
      _in_pattern(Strings::sprintf(_var_pattern, "(?:attribute|in)").c_str()), _out_pattern(Strings::sprintf(_var_pattern, "(?:varying|out)").c_str()),
      _in_out_pattern(Strings::sprintf(_var_pattern, "(?:varying|in)").c_str())
{
    if(version != Ark::AUTO)
        chooseGLVersion(version);
}

void RenderEngine::initialize()
{
    if(_version == Ark::AUTO)
    {
        int glMajorVersion = 0, glMinorVersion = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
        glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
        if(glMajorVersion != 0)
            chooseGLVersion(static_cast<Ark::RenderEngineVersion>(glMajorVersion * 10 + glMinorVersion));
        else
            chooseGLVersion(Ark::OPENGL_20);
    }
}

Ark::RenderEngineVersion RenderEngine::version() const
{
    return _version;
}

uint32_t RenderEngine::getGLSLVersion() const
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

const String& RenderEngine::inTypeName() const
{
    return _in_type_name;
}

const String& RenderEngine::outTypeName() const
{
    return _out_type_name;
}

const String& RenderEngine::fragmentName() const
{
    return _fragment_name;
}

const std::regex& RenderEngine::inPattern() const
{
    return _in_pattern;
}

const std::regex& RenderEngine::outPattern() const
{
    return _out_pattern;
}

const std::regex& RenderEngine::inOutPattern() const
{
    return _in_out_pattern;
}

const std::map<String, String>& RenderEngine::annotations() const
{
    return _annotations;
}

sp<RenderView> RenderEngine::createRenderView(const Viewport& viewport) const
{
    return _render_view_factory->createRenderView(viewport);
}

sp<GLSnippet> RenderEngine::createCoreGLSnippet(const sp<GLResourceManager>& glResourceManager, const sp<GLShader>& shader, const GLBuffer& arrayBuffer) const
{
    return _gl_procedure_factory->createCoreGLSnippet(glResourceManager, shader, arrayBuffer);
}

void RenderEngine::chooseGLVersion(Ark::RenderEngineVersion version)
{
    DCHECK(version != Ark::AUTO, "Cannot set OpenGL version to \"auto\" manually.");
    LOGD("Choose GLVersion = %d", version);
    if(version == Ark::OPENGL_20 || version == Ark::OPENGL_21)
    {
        _annotations["vert.in"] = "attribute";
        _annotations["vert.out"] = "varying";
        _annotations["frag.in"] = "varying";
        _annotations["frag.out"] = "varying";
        _annotations["frag.color"] = "gl_FragColor";
        _gl_procedure_factory = sp<gles20::GLProcedureFactoryGLES20>::make();
    }
    else
    {
        _annotations["vert.in"] = "in";
        _annotations["vert.out"] = "out";
        _annotations["frag.in"] = "in";
        _annotations["frag.out"] = "out";
        _annotations["frag.color"] = "v_FragColor";
        _gl_procedure_factory = sp<gles30::GLProcedureFactoryGLES30>::make();
    }
    _version = version;
}

}
