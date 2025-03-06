#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/collection/table.h"

#include "renderer/forwarding.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader_preprocessor.h"

namespace ark {

class ARK_API PipelineConfiguration {
public:
    PipelineConfiguration(sp<PipelineBuildingContext> buildingContext);

    void addSnippet(sp<Snippet> snippet);
    const sp<Snippet>& snippet() const;
    void initialize(const Shader& shader);

    const sp<PipelineLayout>& pipelineLayout() const;

    void preCompile(GraphicsContext& graphicsContext);

    Map<Enum::ShaderStageBit, ShaderPreprocessor::Stage> getPreprocessedStages(const RenderEngineContext& renderEngineContext) const;

    Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>> makeBindingSamplers() const;
    Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>> makeBindingImages() const;

private:
    sp<PipelineBuildingContext> _building_context;
    sp<PipelineLayout> _pipeline_layout;
    sp<Snippet> _snippet;

    Vector<ShaderPreprocessor::Stage> _stages;

    Table<String, sp<Texture>> _predefined_samplers;
    Table<String, sp<Texture>> _predefined_images;

    Map<String, String> _definitions;

    friend class PipelineBuildingContext;
    friend class Shader;
};

}
