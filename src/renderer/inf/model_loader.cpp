#include "renderer/inf/model_loader.h"

namespace ark {

ModelLoader::ModelLoader(Enum::RenderMode renderMode)
    : _render_mode(renderMode) {
}

Enum::RenderMode ModelLoader::renderMode() const
{
    return _render_mode;
}

}
