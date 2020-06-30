#ifndef ARK_RENDERER_BASE_PIPELINE_BUILDING_CONTEXT_H_
#define ARK_RENDERER_BASE_PIPELINE_BUILDING_CONTEXT_H_

#include <map>
#include <set>
#include <vector>

#include "core/base/string.h"
#include "core/base/string_buffer.h"
#include "core/collection/table.h"
#include "core/types/owned_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_preprocessor.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/uniform.h"

namespace ark {

class PipelineBuildingContext {
public:
    PipelineBuildingContext();
    PipelineBuildingContext(sp<String> vertex, sp<String> fragment);

    void loadManifest(const document& manifest, BeanFactory& factory, const Scope& args);

    void initialize();

    void setupUniforms();

    sp<PipelineInput> _input;
    sp<Snippet> _snippet;

    std::map<String, Attribute> _attributes;
    Table<String, sp<Uniform>> _uniforms;
    Table<String, sp<Texture>> _samplers;
    Table<String, Buffer> _ssbos;

    std::set<String> _input_vars;

    const std::map<Shader::Stage, op<ShaderPreprocessor>>& stages() const;

    void addAttribute(const String& name, const String& type);
    void addSnippet(const sp<Snippet>& snippet);
    void addUniform(const String& name, Uniform::Type type, uint32_t length, const sp<Flatable>& flatable, int32_t binding);
    void addUniform(const sp<Uniform>& uniform);

    void addInputAttribute(const String& name, const String& type);
    Attribute& addPredefinedAttribute(const String& name, const String& type, Shader::Stage stage);

    bool hasStage(Shader::Stage shaderStage) const;
    const op<ShaderPreprocessor>& getStage(Shader::Stage shaderStage) const;
    const op<ShaderPreprocessor>& addStage(sp<String> source, Shader::Stage shaderStage, Shader::Stage preShaderStage);

private:
    Attribute makePredefinedAttribute(const String& name, const String& type);

    void initializePipelines();

    void loadPredefinedAttribute(const document& manifest);
    void loadPredefinedUniform(BeanFactory& factory, const Scope& args, const document& manifest);
    void loadPredefinedSampler(BeanFactory& factory, const Scope& args, const document& manifest);
    void loadPredefinedBuffer(BeanFactory& factory, const Scope& args, const document& manifest);

    std::map<Shader::Stage, op<ShaderPreprocessor>> _stages;
};

}

#endif
