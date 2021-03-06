#ifndef ARK_RENDERER_OPENGL_PIPELINE_FACTORY_PIPELINE_FACTORY_OPENGL_H_
#define ARK_RENDERER_OPENGL_PIPELINE_FACTORY_PIPELINE_FACTORY_OPENGL_H_

#include "core/forwarding.h"

#include "renderer/base/shader.h"
#include "renderer/inf/pipeline_factory.h"

namespace ark {
namespace opengl {

class PipelineFactoryOpenGL : public PipelineFactory {
public:

    virtual sp<Pipeline> buildPipeline(GraphicsContext& graphicsContext, const PipelineBindings& bindings) override;

};

}
}

#endif
