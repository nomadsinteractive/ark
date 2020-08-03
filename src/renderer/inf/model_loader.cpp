#include "renderer/inf/model_loader.h"

#include "core/util/conversions.h"

namespace ark {

ModelLoader::ModelLoader(ModelLoader::RenderMode renderMode)
    : _render_mode(renderMode) {
}

ModelLoader::RenderMode ModelLoader::renderMode() const
{
    return _render_mode;
}

template<> ARK_API ModelLoader::RenderMode Conversions::to<String, ModelLoader::RenderMode>(const String& str)
{
    if(str == "lines")
        return ModelLoader::RENDER_MODE_LINES;
    if(str == "points")
        return ModelLoader::RENDER_MODE_POINTS;
    if(str == "triangles")
        return ModelLoader::RENDER_MODE_TRIANGLES;
    DCHECK(str == "triangle_strip", "Unknow RenderMode: %s, possible values are: [lines, points, triangles, triangle_strip]", str.c_str());
    if(str == "triangle_strip")
        return ModelLoader::RENDER_MODE_TRIANGLE_STRIP;
    return ModelLoader::RENDER_MODE_NONE;
}


}
