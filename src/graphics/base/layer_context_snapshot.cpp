#include "graphics/base/layer_context_snapshot.h"

#include "renderer/base/pipeline_bindings.h"


namespace ark {

const PipelineLayout& LayerContextSnapshot::pipelineInput() const
{
    return _render_layer_stub->_pipeline_bindings->pipelineLayout();
}

const sp<ModelLoader>& LayerContextSnapshot::modelLoader() const
{
    return _render_layer_stub->_model_loader;
}

}
