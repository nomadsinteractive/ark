#include "renderer/base/render_engine_context.h"

#include "renderer/inf/snippet_factory.h"

namespace ark {

RenderEngineContext::RenderEngineContext(Ark::RendererVersion version, Ark::RendererCoordinateSystem coordinateSystem, const Viewport& viewport)
    : _version(version), _coordinate_system(coordinateSystem), _viewport(viewport)
{
}

Ark::RendererVersion RenderEngineContext::version() const
{
    return _version;
}

void RenderEngineContext::setVersion(Ark::RendererVersion version)
{
    _version = version;
}

Ark::RendererCoordinateSystem RenderEngineContext::coordinateSystem() const
{
    return _coordinate_system;
}

const std::map<String, String>& RenderEngineContext::annotations() const
{
    return _annotations;
}

std::map<String, String>& RenderEngineContext::annotations()
{
    return _annotations;
}

const Viewport& RenderEngineContext::viewport() const
{
    return _viewport;
}

void RenderEngineContext::setViewport(const Viewport& viewport)
{
    _viewport = viewport;
    _display_unit = V2(_display_resolution.width / _viewport.width(), _display_resolution.height / _viewport.height());
}

const RenderEngineContext::Resolution& RenderEngineContext::displayResolution() const
{
    return _display_resolution;
}

void RenderEngineContext::setDisplayResolution(const Resolution& resolution)
{
    _display_resolution = resolution;
    _display_unit = V2(_display_resolution.width / _viewport.width(), _display_resolution.height / _viewport.height());
}

const V2& RenderEngineContext::displayUnit() const
{
    return _display_unit;
}

uint32_t RenderEngineContext::getGLSLVersion() const
{
    switch(_version) {
    case Ark::RENDERER_VERSION_OPENGL_20:
        return 110;
    case Ark::RENDERER_VERSION_OPENGL_21:
        return 120;
    case Ark::RENDERER_VERSION_OPENGL_30:
        return 130;
    case Ark::RENDERER_VERSION_OPENGL_31:
        return 140;
    case Ark::RENDERER_VERSION_OPENGL_32:
        return 150;
    case Ark::RENDERER_VERSION_OPENGL_33:
    case Ark::RENDERER_VERSION_OPENGL_40:
    case Ark::RENDERER_VERSION_OPENGL_41:
    case Ark::RENDERER_VERSION_OPENGL_42:
    case Ark::RENDERER_VERSION_OPENGL_43:
    case Ark::RENDERER_VERSION_OPENGL_44:
    case Ark::RENDERER_VERSION_OPENGL_45:
    case Ark::RENDERER_VERSION_OPENGL_46:
        return static_cast<uint32_t>(_version) * 10;
    default:
        break;
    }
    FATAL("Unsupported OpenGL version: %d", _version);
    return 110;
}

const RenderEngineContext::Info& RenderEngineContext::info() const
{
    return _info;
}

RenderEngineContext::Info& RenderEngineContext::info()
{
    return _info;
}

void RenderEngineContext::setSnippetFactory(sp<SnippetFactory> snippetfactory)
{
    _snippet_factory = std::move(snippetfactory);
}

const sp<SnippetFactory>& RenderEngineContext::snippetFactory() const
{
    DCHECK(_snippet_factory, "Uninitialized RenderEngineContext");
    return _snippet_factory;
}

}
