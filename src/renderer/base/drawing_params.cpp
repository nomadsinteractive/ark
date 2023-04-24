#include "renderer/base/drawing_params.h"

#include "graphics/base/camera.h"
#include "graphics/inf/render_command.h"

#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/snippet_delegate.h"
#include "renderer/inf/pipeline.h"

namespace ark {

DrawingParams::DrawElements::DrawElements(uint32_t start)
    : _start(start)
{
}

DrawingParams::DrawingParams()
    : _draw_elements(0)
{
}

DrawingParams::DrawingParams(DrawingParams&& other)
{
    if(other._draw_elements.isActive())
        _draw_elements = std::move(other._draw_elements);
    else if(other._draw_elements_instanced.isActive())
        new(&_draw_elements_instanced) auto(std::move(other._draw_elements_instanced));
    else if(other._draw_multi_elements_indirect.isActive())
        new(&_draw_multi_elements_indirect) auto(std::move(other._draw_multi_elements_indirect));
    else
        DFATAL("Shouldn't be here");
}

DrawingParams::DrawingParams(const DrawingParams& other)
{
    if(other._draw_elements.isActive())
        _draw_elements = other._draw_elements;
    else if(other._draw_elements_instanced.isActive())
        new(&_draw_elements_instanced) auto(other._draw_elements_instanced);
    else if(other._draw_multi_elements_indirect.isActive())
        new(&_draw_multi_elements_indirect) auto(other._draw_multi_elements_indirect);
    else
        DFATAL("Shouldn't be here");
}

DrawingParams::DrawingParams(DrawElements drawElements)
    : _draw_elements(std::move(drawElements))
{
}

DrawingParams::DrawingParams(DrawingParams::DrawElementsInstanced drawElementsInstanced)
    : _draw_elements_instanced(std::move(drawElementsInstanced))
{
}

DrawingParams::DrawingParams(DrawMultiElementsIndirect drawMultiElementsIndirect)
    : _draw_multi_elements_indirect(std::move(drawMultiElementsIndirect))
{
}

DrawingParams::~DrawingParams()
{
    if(_draw_elements.isActive())
        _draw_elements.~DrawElements();
    else if(_draw_elements_instanced.isActive())
        _draw_elements_instanced.~DrawElementsInstanced();
    else if(_draw_multi_elements_indirect.isActive())
        _draw_multi_elements_indirect.~DrawMultiElementsIndirect();
    else
        DFATAL("Shouldn't be here");
}

DrawingParams& DrawingParams::operator =(const DrawingParams& other)
{
    if(other._draw_elements.isActive())
        _draw_elements = other._draw_elements;
    else if(other._draw_elements_instanced.isActive())
        assign(_draw_elements_instanced, other._draw_elements_instanced);
    else if(other._draw_multi_elements_indirect.isActive())
        assign(_draw_multi_elements_indirect, other._draw_multi_elements_indirect);
    else
        DFATAL("Shouldn't be here");
    return *this;
}

DrawingParams& DrawingParams::operator =(DrawingParams&& other)
{
    if(other._draw_elements.isActive())
        _draw_elements = other._draw_elements;
    else if(other._draw_elements_instanced.isActive())
        assign(_draw_elements_instanced, std::move(other._draw_elements_instanced));
    else if(other._draw_multi_elements_indirect.isActive())
        assign(_draw_multi_elements_indirect, std::move(other._draw_multi_elements_indirect));
    else
        DFATAL("Shouldn't be here");
    return *this;
}

DrawingParams::DrawElementsInstanced::DrawElementsInstanced(uint32_t start, uint32_t count, std::vector<std::pair<uint32_t, Buffer::Snapshot>> snapshots)
    : _count(count), _start(start), _divided_buffer_snapshots(std::move(snapshots))
{
}

DrawingParams::DrawMultiElementsIndirect::DrawMultiElementsIndirect(std::vector<std::pair<uint32_t, Buffer::Snapshot>> snapshots, Buffer::Snapshot indirectCmds, uint32_t indirectCmdCount)
    : _divided_buffer_snapshots(std::move(snapshots)), _indirect_cmds(std::move(indirectCmds)), _indirect_cmd_count(indirectCmdCount)
{
}

}
