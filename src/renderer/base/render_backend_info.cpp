#include "renderer/base/render_backend_info.h"

#include "renderer/inf/snippet_factory.h"

namespace ark {

RenderBackendInfo::RenderBackendInfo(const ApplicationManifest::Renderer& renderer, const Viewport& viewport, const enums::CoordinateSystem viewportCoordinateSystem, const enums::CoordinateSystem ndcCoordinateSystem, const enums::NDCDepthRange ndcDepthRange)
    : _renderer(renderer), _viewport(viewport), _viewport_coordinate_system(viewportCoordinateSystem), _ndc_coordinate_system(ndcCoordinateSystem), _ndc_depth_range(ndcDepthRange)
{
}

const ApplicationManifest::Renderer& RenderBackendInfo::renderer() const
{
    return _renderer;
}

enums::RendererVersion RenderBackendInfo::version() const
{
    return _renderer._version;
}

void RenderBackendInfo::setVersion(const enums::RendererVersion version)
{
    _renderer._version = version;
}

const Viewport& RenderBackendInfo::viewport() const
{
    return _viewport;
}

void RenderBackendInfo::setViewport(const Viewport& viewport)
{
    _viewport = viewport;
    _display_unit = V2(_display_resolution.width / _viewport.width(), _display_resolution.height / _viewport.height());
}

enums::CoordinateSystem RenderBackendInfo::viewportCoordinateSystem() const
{
    return _viewport_coordinate_system;
}

enums::CoordinateSystem RenderBackendInfo::ndcCoordinateSystem() const
{
    return _ndc_coordinate_system;
}

enums::NDCDepthRange RenderBackendInfo::ndcDepthRange() const
{
    return _ndc_depth_range;
}

const RenderBackendInfo::Resolution& RenderBackendInfo::displayResolution() const
{
    return _display_resolution;
}

void RenderBackendInfo::setDisplayResolution(const Resolution& displayResolution)
{
    _display_resolution = displayResolution;
    _display_unit = V2(_display_resolution.width / _viewport.width(), _display_resolution.height / _viewport.height());
}

const V2& RenderBackendInfo::displayUnit() const
{
    return _display_unit;
}

uint32_t RenderBackendInfo::getGLSLVersion() const
{
    switch(_renderer._version) {
    case enums::RENDERER_VERSION_OPENGL_30:
        return 130;
    case enums::RENDERER_VERSION_OPENGL_31:
        return 140;
    case enums::RENDERER_VERSION_OPENGL_32:
        return 150;
    case enums::RENDERER_VERSION_OPENGL_33:
    case enums::RENDERER_VERSION_OPENGL_40:
    case enums::RENDERER_VERSION_OPENGL_41:
    case enums::RENDERER_VERSION_OPENGL_42:
    case enums::RENDERER_VERSION_OPENGL_43:
    case enums::RENDERER_VERSION_OPENGL_44:
    case enums::RENDERER_VERSION_OPENGL_45:
    case enums::RENDERER_VERSION_OPENGL_46:
        return static_cast<uint32_t>(_renderer._version - enums::RENDERER_VERSION_OPENGL) * 10;
    default:
        break;
    }
    FATAL("Unsupported OpenGL version: %d", _renderer._version);
    return 110;
}

Traits& RenderBackendInfo::traits()
{
    return _traits;
}

const Traits& RenderBackendInfo::traits() const
{
    return _traits;
}

void RenderBackendInfo::setSnippetFactory(sp<SnippetFactory> snippetfactory)
{
    _snippet_factory = std::move(snippetfactory);
}

const sp<SnippetFactory>& RenderBackendInfo::snippetFactory() const
{
    DCHECK(_snippet_factory, "Uninitialized RenderEngineInfo");
    return _snippet_factory;
}

}
