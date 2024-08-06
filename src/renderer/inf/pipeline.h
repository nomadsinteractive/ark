#pragma once

#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/inf/resource.h"

namespace ark {

class ARK_API PipelineDrawCommand {
public:
    virtual ~PipelineDrawCommand() = default;

    virtual void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) = 0;
};

class ARK_API PipelineOperation : public PipelineDrawCommand {
public:
    ~PipelineOperation() override = default;

    virtual void bind(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) = 0;
    virtual void compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext) = 0;
};

class ARK_API Pipeline : public Resource, public PipelineOperation {
public:
    ~Pipeline() override = default;
};

}
