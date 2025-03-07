#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/collection/table.h"

#include "renderer/forwarding.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader_preprocessor.h"

namespace ark {

class PipelineConfiguration {
public:
    PipelineConfiguration(Camera camera, sp<PipelineBuildingContext> buildingContext, sp<Snippet> snippet = nullptr);

    void addSnippet(sp<Snippet> snippet);
    const sp<Snippet>& snippet() const;

    const sp<PipelineLayout>& pipelineLayout() const;

    void preCompile(GraphicsContext& graphicsContext, const PipelineDescriptor& pipelineDescriptor);

    Map<Enum::ShaderStageBit, ShaderPreprocessor::Stage> getPreprocessedStages(const RenderEngineContext& renderEngineContext) const;

    Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>> makeBindingSamplers() const;
    Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>> makeBindingImages() const;

private:
    void initialize();

private:
    Camera _camera;
    sp<Snippet> _snippet;
    sp<PipelineBuildingContext> _building_context;

    sp<PipelineLayout> _pipeline_layout;

    Vector<ShaderPreprocessor::Stage> _stages;

    Table<String, sp<Texture>> _predefined_samplers;
    Table<String, sp<Texture>> _predefined_images;

    Map<String, String> _definitions;

    friend class PipelineBuildingContext;
    friend class PipelineDescriptor;
    friend class Shader;
};

}
