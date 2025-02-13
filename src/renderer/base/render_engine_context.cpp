#include "renderer/base/render_engine_context.h"

#include "renderer/inf/snippet_factory.h"

namespace ark {

RenderEngineContext::RenderEngineContext(const ApplicationManifest::Renderer& renderer, const Viewport& viewport)
    : _renderer(renderer), _viewport(viewport), _definitions{{"camera.uVP", "u_VP"}, {"camera.uView", "u_View"}, {"camera.uProjection", "u_Projection"}}
{
}

const ApplicationManifest::Renderer& RenderEngineContext::renderer() const
{
    return _renderer;
}

Ark::RendererVersion RenderEngineContext::version() const
{
    return _renderer._version;
}

void RenderEngineContext::setVersion(Ark::RendererVersion version)
{
    _renderer._version = version;
}

const std::map<String, String>& RenderEngineContext::definitions() const
{
    return _definitions;
}

std::map<String, String>& RenderEngineContext::definitions()
{
    return _definitions;
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
    switch(_renderer._version) {
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
        return static_cast<uint32_t>(_renderer._version) * 10;
    default:
        break;
    }
    FATAL("Unsupported OpenGL version: %d", _renderer._version);
    return 110;
}

Traits& RenderEngineContext::traits()
{
    return _traits;
}

const Traits& RenderEngineContext::traits() const
{
    return _traits;
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
