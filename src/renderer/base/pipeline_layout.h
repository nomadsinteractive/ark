#ifndef ARK_RENDERER_BASE_PIPELINE_LAYOUT_H_
#define ARK_RENDERER_BASE_PIPELINE_LAYOUT_H_

#include <map>

#include "core/base/api.h"
#include "core/base/string.h"

#include "renderer/forwarding.h"
#include "renderer/base/attribute.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_preprocessor.h"
#include "renderer/base/uniform.h"

namespace ark {

class PipelineLayout {
public:
    PipelineLayout(const sp<PipelineBuildingContext>& buildingContext);

    const sp<Snippet>& snippet() const;

    void preCompile(GraphicsContext& graphicsContext);

    const sp<PipelineInput>& input() const;

    std::map<Shader::Stage, String> getPreprocessedShaders(const RenderEngineContext& renderEngineContext) const;

    const ShaderPreprocessor::Preprocessor& vertex() const;
    const ShaderPreprocessor::Preprocessor& fragment() const;

    const Table<String, sp<Texture>>& samplers() const;

private:
    void initialize(const Camera& camera);

    void addUniform(const String& name, const sp<Flatable>& flatable);

private:
    sp<PipelineBuildingContext> _building_context;

    sp<PipelineInput> _input;
    sp<Snippet> _snippet;

    ShaderPreprocessor::Preprocessor _vertex;
    ShaderPreprocessor::Preprocessor _fragment;

    Table<String, sp<Texture>> _samplers;

    friend class Shader;
    friend class ShaderBindings;
    friend class PipelineBuildingContext;
    friend class ShaderPreprocessor;

};

}

#endif
