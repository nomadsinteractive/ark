#pragma once

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

namespace ark {

class PipelineLayout {
public:
    PipelineLayout(sp<PipelineBuildingContext> buildingContext);

    void initialize();

    const sp<Snippet>& snippet() const;
    const sp<PipelineInput>& input() const;

    void preCompile(GraphicsContext& graphicsContext);

    std::map<PipelineInput::ShaderStage, String> getPreprocessedShaders(const RenderEngineContext& renderEngineContext) const;

    size_t colorAttachmentCount() const;

    const Table<String, sp<Texture>>& samplers() const;
    const std::vector<sp<Texture>>& images() const;

private:
    void tryBindUniformMatrix(const ShaderPreprocessor& shaderPreprocessor, String name, sp<Mat4> matrix);
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
    friend class Shader;

};

}
