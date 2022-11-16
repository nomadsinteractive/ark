#ifndef ARK_RENDERER_BASE_PIPELINE_LAYOUT_H_
#define ARK_RENDERER_BASE_PIPELINE_LAYOUT_H_

#include <map>
#include <vector>

#include "core/base/api.h"
#include "core/base/string.h"

#include "renderer/forwarding.h"
#include "renderer/base/attribute.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader_preprocessor.h"
#include "renderer/base/uniform.h"

namespace ark {

class PipelineLayout {
public:
    PipelineLayout(const sp<PipelineBuildingContext>& buildingContext);

    const sp<Snippet>& snippet() const;

    void preCompile(GraphicsContext& graphicsContext);

    const sp<PipelineInput>& input() const;

    std::map<PipelineInput::ShaderStage, String> getPreprocessedShaders(const RenderEngineContext& renderEngineContext) const;

    uint32_t colorAttachmentCount() const;

private:
    void initialize(const Camera& camera);
    void tryBindUniform(const ShaderPreprocessor& shaderPreprocessor, const String& name, const sp<Input>& input);

    std::vector<sp<Texture>> makeBindingSamplers() const;
    std::vector<sp<Texture>> makeBindingImages() const;

private:
    sp<PipelineBuildingContext> _building_context;

    sp<PipelineInput> _input;
    sp<Snippet> _snippet;

    std::map<PipelineInput::ShaderStage, ShaderPreprocessor::Preprocessed> _preprocessed_stages;

    uint32_t _color_attachment_count;

    friend class Shader;
    friend class ShaderBindings;
    friend class PipelineBuildingContext;
    friend class PipelineBindings;
    friend class ShaderPreprocessor;

};

}

#endif
