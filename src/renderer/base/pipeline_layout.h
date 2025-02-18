#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/collection/table.h"

#include "renderer/forwarding.h"
#include "renderer/base/shader_layout.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader_preprocessor.h"

namespace ark {

class ARK_API PipelineLayout {
public:
    PipelineLayout(sp<PipelineBuildingContext> buildingContext);

    void addSnippet(sp<Snippet> snippet);
    const sp<Snippet>& snippet() const;
    void initialize(const Shader& shader);

    const sp<ShaderLayout>& shaderLayout() const;

    void preCompile(GraphicsContext& graphicsContext);

    Map<Enum::ShaderStageBit, ShaderPreprocessor::Stage> getPreprocessedStages(const RenderEngineContext& renderEngineContext) const;

    size_t colorAttachmentCount() const;

    Vector<std::pair<sp<Texture>, ShaderLayout::DescriptorSet>> makeBindingSamplers() const;
    Vector<std::pair<sp<Texture>, ShaderLayout::DescriptorSet>> makeBindingImages() const;

private:
    sp<PipelineBuildingContext> _building_context;
    sp<ShaderLayout> _shader_layout;
    sp<Snippet> _snippet;

    Vector<ShaderPreprocessor::Stage> _preprocessed_stages;

    Table<String, sp<Texture>> _predefined_samplers;
    Table<String, sp<Texture>> _predefined_images;

    size_t _color_attachment_count;

    Map<String, String> _definitions;

    friend class PipelineBuildingContext;
    friend class Shader;
};

}
