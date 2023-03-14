#include "renderer/base/drawing_context_params.h"

#include "graphics/base/camera.h"
#include "graphics/inf/render_command.h"

#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/snippet_delegate.h"
#include "renderer/inf/pipeline.h"

namespace ark {

DrawingContextParams::DrawElements::DrawElements(uint32_t start, uint32_t count)
    : _count(count), _start(start)
{
}

DrawingContextParams::Parameters::Parameters()
    : _draw_elements(0, 0)
{
}

DrawingContextParams::Parameters::Parameters(DrawingContextParams::Parameters&& other)
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

DrawingContextParams::Parameters::Parameters(const DrawingContextParams::Parameters& other)
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

DrawingContextParams::Parameters::Parameters(const DrawingContextParams::DrawElements& drawElements)
    : _draw_elements(drawElements)
{
}

DrawingContextParams::Parameters::Parameters(DrawingContextParams::DrawElementsInstanced drawElementsInstanced)
    : _draw_elements_instanced(std::move(drawElementsInstanced))
{
}

DrawingContextParams::Parameters::Parameters(DrawMultiElementsIndirect drawMultiElementsIndirect)
    : _draw_multi_elements_indirect(std::move(drawMultiElementsIndirect))
{
}

DrawingContextParams::Parameters::~Parameters()
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

DrawingContextParams::Parameters& DrawingContextParams::Parameters::operator =(const DrawingContextParams::Parameters& other)
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

DrawingContextParams::Parameters& DrawingContextParams::Parameters::operator =(DrawingContextParams::Parameters&& other)
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

DrawingContextParams::DrawElementsInstanced::DrawElementsInstanced(uint32_t start, uint32_t count, int32_t instanceCount, std::vector<std::pair<uint32_t, Buffer::Snapshot>> snapshots)
    : _count(count), _start(start), _instance_count(instanceCount), _instanced_array_snapshots(std::move(snapshots))
{
}

DrawingContextParams::DrawMultiElementsIndirect::DrawMultiElementsIndirect(std::vector<std::pair<uint32_t, Buffer::Snapshot>> snapshots, Buffer::Snapshot indirectCmds, uint32_t drawCount)
    : _instanced_array_snapshots(std::move(snapshots)), _indirect_cmds(std::move(indirectCmds)), _draw_count(drawCount)
{
}

}
