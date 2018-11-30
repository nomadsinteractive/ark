#ifndef ARK_RENDERER_OPENGL_PIPELINE_FACTORY_PIPELINE_FACTORY_OPENGL_H_
#define ARK_RENDERER_OPENGL_PIPELINE_FACTORY_PIPELINE_FACTORY_OPENGL_H_

#include "core/forwarding.h"

#include "renderer/inf/pipeline_factory.h"

#include "platform/gl/gl.h"

namespace ark {
namespace opengl {

class PipelineFactoryOpenGL : public PipelineFactory {
public:
    PipelineFactoryOpenGL();

    virtual sp<Pipeline> buildPipeline(GraphicsContext& graphicsContext, const PipelineLayout& pipelineLayout) override;
    virtual sp<RenderCommand> buildRenderCommand(ObjectPool& objectPool, DrawingContext drawingContext, const sp<Shader>& shader, RenderModel::Mode mode, int32_t count) override;

private:
    GLenum _models[RenderModel::RENDER_MODE_COUNT];
};

}
}

#endif
