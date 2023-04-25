#include "renderer/base/drawing_params.h"

namespace ark {

DrawingParams::DrawingParams(DrawElements drawElements)
    : _params(std::move(drawElements))
{
}

DrawingParams::DrawingParams(DrawingParams::DrawElementsInstanced drawElementsInstanced)
    : _params(std::move(drawElementsInstanced))
{
}

DrawingParams::DrawingParams(DrawMultiElementsIndirect drawMultiElementsIndirect)
    : _params(std::move(drawMultiElementsIndirect))
{
}

const DrawingParams::DrawElements& DrawingParams::drawElements() const
{
    DASSERT(std::holds_alternative<DrawElements>(_params));
    return std::get<DrawElements>(_params);
}

const DrawingParams::DrawElementsInstanced& DrawingParams::drawElementsInstanced() const
{
    DASSERT(std::holds_alternative<DrawElementsInstanced>(_params));
    return std::get<DrawElementsInstanced>(_params);
}

const DrawingParams::DrawMultiElementsIndirect& DrawingParams::drawMultiElementsIndirect() const
{
    DASSERT(std::holds_alternative<DrawMultiElementsIndirect>(_params));
    return std::get<DrawMultiElementsIndirect>(_params);
}

}
