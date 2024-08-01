#include "graphics/base/layer_context_snapshot.h"

#include "renderer/base/pipeline_bindings.h"


namespace ark {

const PipelineInput& LayerContextSnapshot::pipelineInput() const
{
    return _render_layer._stub->_pipeline_bindings->pipelineInput();
}

}
