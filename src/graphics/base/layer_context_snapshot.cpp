#include "graphics/base/layer_context_snapshot.h"

#include "renderer/base/shader_bindings.h"


namespace ark {

const PipelineInput& LayerContextSnapshot::pipelineInput() const
{
    return _render_layer._stub->_shader_bindings->pipelineInput();
}

}
