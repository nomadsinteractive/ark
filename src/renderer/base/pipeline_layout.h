#pragma once

#include <map>
#include <vector>

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/collection/table.h"

#include "renderer/forwarding.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader_preprocessor.h"

namespace ark {

class ARK_API PipelineLayout {
public:
    PipelineLayout(sp<PipelineBuildingContext> buildingContext);

    void addSnippet(sp<Snippet> snippet);
    const sp<Snippet>& snippet() const;
    void initialize();

    const sp<PipelineInput>& input() const;

    void preCompile(GraphicsContext& graphicsContext);

    std::map<Enum::ShaderStageBit, String> getPreprocessedShaders(const RenderEngineContext& renderEngineContext) const;
    const std::vector<PipelineInput::BindingSet>& samplers() const;
    const std::vector<PipelineInput::BindingSet>& images() const;

    size_t colorAttachmentCount() const;

    std::vector<std::pair<sp<Texture>, PipelineInput::BindingSet>> makeBindingSamplers() const;
    std::vector<std::pair<sp<Texture>, PipelineInput::BindingSet>> makeBindingImages() const;

private:
    sp<PipelineBuildingContext> _building_context;
    sp<PipelineInput> _input;
    sp<Snippet> _snippet;

    std::vector<ShaderPreprocessor::Stage> _preprocessed_stages;
    std::vector<PipelineInput::BindingSet> _samplers;
    std::vector<PipelineInput::BindingSet> _images;

    size_t _color_attachment_count;

    std::map<String, String> _definitions;

    friend class PipelineBuildingContext;
    friend class Shader;
};

}
