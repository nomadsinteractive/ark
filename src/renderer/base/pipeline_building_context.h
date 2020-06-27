#ifndef ARK_RENDERER_BASE_PIPELINE_BUILDING_CONTEXT_H_
#define ARK_RENDERER_BASE_PIPELINE_BUILDING_CONTEXT_H_

#include <map>
#include <set>
#include <vector>

#include "core/base/string.h"
#include "core/base/string_buffer.h"
#include "core/collection/table.h"

#include "renderer/forwarding.h"
#include "renderer/base/shader_preprocessor.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/uniform.h"
#include "renderer/base/shader.h"

namespace ark {

class PipelineBuildingContext {
public:
    PipelineBuildingContext(const String& vertex, const String& fragment);
    PipelineBuildingContext(const String& vertex, const String& fragment, BeanFactory& factory, const Scope& args, const document& manifest);

    void initialize();

    void setupUniforms();

    sp<PipelineInput> _input;
    sp<Snippet> _snippet;

    ShaderPreprocessor _vertex;
    ShaderPreprocessor _fragment;

    std::map<Shader::Stage, ShaderPreprocessor> _stages;

    std::map<String, Attribute> _attributes;
    Table<String, sp<Uniform>> _uniforms;
    Table<String, sp<Texture>> _samplers;

    std::set<String> _input_vars;

    void addAttribute(const String& name, const String& type);
    void addSnippet(const sp<Snippet>& snippet);
    void addUniform(const String& name, Uniform::Type type, uint32_t length, const sp<Flatable>& flatable, int32_t binding);
    void addUniform(const sp<Uniform>& uniform);

    void addInputAttribute(const String& name, const String& type);
    Attribute& addPredefinedAttribute(const String& name, const String& type, uint32_t scopes);

private:
    Attribute makePredefinedAttribute(const String& name, const String& type);

    void loadClassicalPipeline(const String& vertex, const String& fragment);
    void loadPredefinedParam(BeanFactory& factory, const Scope& args, const document& manifest);

    void loadPredefinedAttribute(const document& manifest);
    void loadPredefinedUniform(BeanFactory& factory, const Scope& args, const document& manifest);
    void loadPredefinedSampler(BeanFactory& factory, const Scope& args, const document& manifest);
};

}

#endif
