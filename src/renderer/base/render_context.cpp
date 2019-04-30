#include "renderer/base/render_context.h"

#include "renderer/inf/snippet_factory.h"

namespace ark {

RenderContext::RenderContext(Ark::RendererVersion version, const Viewport& viewport, float upDirection)
    : _version(version), _viewport(viewport), _up_direction(upDirection)
{
}

Ark::RendererVersion RenderContext::version() const
{
    return _version;
}

void RenderContext::setVersion(Ark::RendererVersion version)
{
    _version = version;
}

const std::map<String, String>& RenderContext::annotations() const
{
    return _annotations;
}

std::map<String, String>& RenderContext::annotations()
{
    return _annotations;
}

const Viewport& RenderContext::viewport() const
{
    return _viewport;
}

void RenderContext::setViewport(const Viewport& viewport)
{
    _viewport = viewport;
}

const V2& RenderContext::resolution() const
{
    return _resolution;
}

void RenderContext::setResolution(const V2& resolution)
{
    _resolution = resolution;
}

uint32_t RenderContext::getGLSLVersion() const
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

const RenderContext::Info& RenderContext::info() const
{
    return _info;
}

RenderContext::Info& RenderContext::info()
{
    return _info;
}

float RenderContext::upDirection() const
{
    return _up_direction;
}

void RenderContext::setSnippetFactory(sp<SnippetFactory> snippetfactory)
{
    _snippet_factory = std::move(snippetfactory);
}

const sp<SnippetFactory>& RenderContext::snippetFactory() const
{
    DCHECK(_snippet_factory, "Uninitialized GLContext");
    return _snippet_factory;
}

}
