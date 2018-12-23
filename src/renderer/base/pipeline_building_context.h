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

namespace ark {

class PipelineBuildingContext {
public:
    PipelineBuildingContext(const String& vertex, const String& fragment);

    void loadPredefinedParam(BeanFactory& factory, const sp<Scope>& args, const document& manifest);

    void initialize();

    sp<PipelineInput> _input;
    sp<Snippet> _snippet;

    ShaderPreprocessor _vertex;
    ShaderPreprocessor _fragment;

    std::map<String, Attribute> _attributes;
    Table<String, Uniform> _uniforms;

    List<std::pair<String, String>> _vertex_in;
    List<std::pair<String, String>> _vertex_out;

    List<std::pair<String, String>> _fragment_in;

    std::map<String, String> _vert_in_declared;

    StringBuffer _vert_main_source;
    StringBuffer _frag_color_modifier;
    StringBuffer _frag_procedures;
    StringBuffer _frag_procedure_calls;

    void addFragmentProcedure(const String& name, const List<std::pair<String, String>>& ins, const String& procedure);

    void preCompile();

    void addAttribute(const String& name, const String& type);
    void addSnippet(const sp<Snippet>& snippet);
    void addUniform(const String& name, Uniform::Type type, const sp<Flatable>& flatable, const sp<Changed>& dirty);
    void addUniform(Uniform uniform);

    Attribute& addPredefinedAttribute(const String& name, const String& type, uint32_t scopes);

private:
    Attribute makePredefinedAttribute(const String& name, const String& type);

    void loadPredefinedAttribute(const document& manifest);
    void loadPredefinedUniform(BeanFactory& factory, const sp<Scope>& args, const document& manifest);

    void doSnippetPrecompile();
    void doPrecompile(String& vertSource, String& fragSource);

    void insertBefore(String& src, const String& statement, const String& str);

};

}

#endif
