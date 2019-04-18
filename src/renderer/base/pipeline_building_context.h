#ifndef ARK_RENDERER_BASE_PIPELINE_BUILDING_CONTEXT_H_
#define ARK_RENDERER_BASE_PIPELINE_BUILDING_CONTEXT_H_

#include <map>

#include "core/base/string.h"
#include "core/base/string_buffer.h"
#include "core/collection/list.h"
#include "core/collection/table.h"

#include "renderer/forwarding.h"
#include "renderer/base/shader_preprocessor.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/uniform.h"
#include "renderer/base/shader.h"

namespace ark {

class PipelineBuildingContext {
public:
    PipelineBuildingContext(const sp<PipelineFactory>& pipelineFactory, const String& vertex, const String& fragment);
    PipelineBuildingContext(const sp<PipelineFactory>& pipelineFactory, const String& vertex, const String& fragment, BeanFactory& factory, const sp<Scope>& args, const document& manifest);

    void initialize();

    int32_t setupBindings();

    sp<Shader::Stub> _shader;
    sp<PipelineInput> _input;
    sp<Snippet> _snippet;

    ShaderPreprocessor _vertex;
    ShaderPreprocessor _fragment;

    std::map<String, Attribute> _attributes;
    Table<String, sp<Uniform>> _uniforms;
    Table<String, sp<Texture>> _samplers;

    List<std::pair<String, String>> _vertex_in;
    List<std::pair<String, String>> _vertex_out;

    List<std::pair<String, String>> _fragment_in;

    std::map<String, String> _vert_in_declared;

    void addAttribute(const String& name, const String& type);
    void addSnippet(const sp<Snippet>& snippet);
    void addUniform(const String& name, Uniform::Type type, const sp<Flatable>& flatable, const sp<Notifier>& notifier, int32_t binding);
    void addUniform(const sp<Uniform>& uniform);

    Attribute& addPredefinedAttribute(const String& name, const String& type, uint32_t scopes);

private:
    Attribute makePredefinedAttribute(const String& name, const String& type);

    void loadPredefinedParam(BeanFactory& factory, const sp<Scope>& args, const document& manifest);

    void loadPredefinedAttribute(const document& manifest);
    void loadPredefinedUniform(BeanFactory& factory, const sp<Scope>& args, const document& manifest);
    void loadPredefinedSampler(BeanFactory& factory, const sp<Scope>& args, const document& manifest);
};

}

#endif
