#pragma once

#include <map>

#include "core/base/api.h"
#include "core/base/enum.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API PipelineFactory {
public:
    virtual ~PipelineFactory() = default;

    virtual sp<Pipeline> buildPipeline(GraphicsContext& graphicsContext, const PipelineDescriptor& pipelineDescriptor, std::map<Enum::ShaderStageBit, String> stages) = 0;

};

}
