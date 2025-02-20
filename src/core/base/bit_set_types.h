#pragma once

#include "core/base/bit_set.h"
#include "core/base/enum.h"

namespace ark {

typedef BitSet<Enum::ShaderStageBit, true> ShaderStageSet;
typedef BitSet<Enum::RenderingBackendBit> RenderingBackendSet;

}
