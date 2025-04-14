#pragma once

#include "core/base/bit_set.h"
#include "core/base/enum.h"

namespace ark::enums {

typedef BitSet<RenderingBackendBit> RenderingBackendSet;
typedef BitSet<ShaderStageBit, true> ShaderStageSet;
typedef BitSet<UploadStrategyBit> UploadStrategy;

}
