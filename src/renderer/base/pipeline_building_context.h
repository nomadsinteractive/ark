#pragma once

#include <map>
#include <set>
#include <vector>

#include "core/base/string.h"
#include "core/base/string_buffer.h"
#include "core/collection/table.h"
#include "core/types/owned_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/shader_preprocessor.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/uniform.h"

namespace ark {

class PipelineBuildingContext {
public:
    PipelineBuildingContext(const sp<RenderController>& renderController);
    PipelineBuildingContext(const sp<RenderController>& renderController, sp<String> vertex, sp<String> fragment);

    void loadManifest(const document& manifest, BeanFactory& factory, const Scope& args);

    void initialize();

    void setupUniforms();

    sp<RenderController> _render_controller;
    sp<PipelineInput> _input;
    sp<Snippet> _snippet;

    std::map<String, Attribute> _attributes;
    std::map<String, sp<StringVar>> _definitions;
    Table<String, sp<Uniform>> _uniforms;
    Table<String, sp<Texture>> _samplers;
    Table<String, sp<Texture>> _images;
    Table<String, Buffer> _ssbos;

    std::set<String> _input_vars;

    const std::map<PipelineInput::ShaderStage, op<ShaderPreprocessor>>& stages() const;

    void addAttribute(String name, String type, uint32_t divisor);
    void addSnippet(const sp<Snippet>& snippet);
    void addUniform(String name, Uniform::Type type, uint32_t length, sp<Uploader> input, int32_t binding);
    void addUniform(sp<Uniform> uniform);

    void addInputAttribute(const String& name, const String& type, uint32_t divisor);
    Attribute& addPredefinedAttribute(const String& name, const String& type, uint32_t divisor,PipelineInput::ShaderStage stage);

    bool hasStage(PipelineInput::ShaderStage shaderStage) const;
    ShaderPreprocessor* tryGetStage(PipelineInput::ShaderStage shaderStage) const;
    const op<ShaderPreprocessor>& getStage(PipelineInput::ShaderStage shaderStage) const;
    const op<ShaderPreprocessor>& addStage(sp<String> source, PipelineInput::ShaderStage shaderStage, PipelineInput::ShaderStage preShaderStage);

    sp<Snippet> makePipelineSnippet() const;
    std::map<String, String> toDefinitions() const;

private:
    Attribute makePredefinedAttribute(const String& name, const String& type);

    void initializePipelines();

    void loadPredefinedAttribute(const document& manifest);
    void loadPredefinedUniform(BeanFactory& factory, const Scope& args, const document& manifest);
    void loadPredefinedSampler(BeanFactory& factory, const Scope& args, const document& manifest);
    void loadPredefinedImage(BeanFactory& factory, const Scope& args, const document& manifest);
    void loadPredefinedBuffer(BeanFactory& factory, const Scope& args, const document& manifest);
    void loadDefinitions(BeanFactory& factory, const Scope& args, const document& manifest);

    std::map<PipelineInput::ShaderStage, op<ShaderPreprocessor>> _stages;
};

}
