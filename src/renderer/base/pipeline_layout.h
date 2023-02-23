#ifndef ARK_RENDERER_BASE_PIPELINE_LAYOUT_H_
#define ARK_RENDERER_BASE_PIPELINE_LAYOUT_H_

#include <map>
#include <vector>

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/collection/table.h"

#include "renderer/forwarding.h"
#include "renderer/base/attribute.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader_preprocessor.h"
#include "renderer/base/uniform.h"

namespace ark {

class PipelineLayout {
public:
    PipelineLayout(const sp<PipelineBuildingContext>& buildingContext, const Camera& camera);

    const sp<Snippet>& snippet() const;

    void preCompile(GraphicsContext& graphicsContext);

    const sp<PipelineInput>& input() const;

    std::map<PipelineInput::ShaderStage, String> getPreprocessedShaders(const RenderEngineContext& renderEngineContext) const;

    size_t colorAttachmentCount() const;

    const Table<String, sp<Texture>>& samplers() const;
    const std::vector<sp<Texture>>& images() const;

private:
    void initialize(const Camera& camera);
    void tryBindUniform(const ShaderPreprocessor& shaderPreprocessor, const String& name, const sp<Input>& input);
    void tryBindCamera(const ShaderPreprocessor& shaderPreprocessor, const Camera& camera);

    Table<String, sp<Texture>> makeBindingSamplers() const;
    std::vector<sp<Texture>> makeBindingImages() const;

private:
    sp<PipelineBuildingContext> _building_context;

    sp<PipelineInput> _input;
    sp<Snippet> _snippet;

    std::map<PipelineInput::ShaderStage, ShaderPreprocessor::Preprocessed> _preprocessed_stages;

    size_t _color_attachment_count;

    Table<String, sp<Texture>> _samplers;
    std::vector<sp<Texture>> _images;
    std::map<String, String> _definitions;

    friend class PipelineBuildingContext;

};

}

#endif
