#include "renderer/base/shader_stage.h"

namespace ark {

void ShaderStage::add(Set stage)
{
    _stages.set(static_cast<Set>(1 << stage), true);
}

bool ShaderStage::has(Set stage) const
{
    return _stages.has(static_cast<Set>(1 << stage));
}

}
