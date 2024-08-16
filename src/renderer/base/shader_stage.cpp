#include "renderer/base/shader_stage.h"

namespace ark {

void ShaderStage::add(BitSet stage)
{
    _stages.set(stage, true);
}

bool ShaderStage::has(BitSet stage) const
{
    return _stages.test(stage);
}

}
