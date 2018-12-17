#ifndef ARK_RENDERER_BASE_SHADER_PREPROCESSOR_H_
#define ARK_RENDERER_BASE_SHADER_PREPROCESSOR_H_

#include <map>

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/base/string_buffer.h"
#include "core/collection/list.h"
#include "core/types/global.h"

#include "renderer/forwarding.h"
#include "renderer/base/attribute.h"
#include "renderer/base/render_engine.h"

namespace ark {

class ShaderPreprocessor {
private:
    enum SnippetType {
        SNIPPET_TYPE_SOURCE,
        SNIPPET_TYPE_PROCEDURE,
        SNIPPET_TYPE_PROCEDURE_CALL,
        SNIPPET_TYPE_MULTIPLY,
        SNIPPET_TYPE_ADD
    };

    struct Snippet {
        Snippet(SnippetType type, const String& src);
        DEFAULT_COPY_AND_ASSIGN(Snippet);

        SnippetType _type;
        String _src;
    };

    struct Function {
        Function(const String& name, const String& params, const String& body);
        DEFAULT_COPY_AND_ASSIGN(Function);

        String _name;
        String _params;
        String _body;
        std::vector<std::pair<String, String>> _ins;
        std::vector<std::pair<String, String>> _outs;
    };

    struct CodeBlock {
        CodeBlock(const String& prefix, const Function& procedure, const String& suffix);
        DEFAULT_COPY_AND_ASSIGN(CodeBlock);

        void parse(PipelineLayout& pipelineLayout);

        bool hasOutParam(const String& name) const;

        String _prefix;
        Function _function;
        String _suffix;
    };

    class Declaration {
    public:
        Declaration(const String& category);

        void declare(const String& type, const String& prefix, const String& name);
        void parse(const String& src, const std::regex& pattern);

        bool dirty() const;
        bool has(const String& name) const;
        String str() const;

        String _category;

        StringBuffer _lines;
        std::map<String, String> _declared;
    };

public:
    enum ShaderType {
        SHADER_TYPE_NONE,
        SHADER_TYPE_VERTEX,
        SHADER_TYPE_FRAGMENT
    };

    static const char* ANNOTATION_VERT_IN;
    static const char* ANNOTATION_VERT_OUT;
    static const char* ANNOTATION_FRAG_IN;
    static const char* ANNOTATION_FRAG_OUT;
    static const char* ANNOTATION_FRAG_COLOR;

    static std::regex _IN_PATTERN;
    static std::regex _OUT_PATTERN;
    static std::regex _IN_OUT_PATTERN;
    static std::regex _UNIFORM_PATTERN;

    class Preprocessor {
    public:
        Preprocessor();
        Preprocessor(ShaderType type, String source);
        DEFAULT_COPY_AND_ASSIGN(Preprocessor);

        String process(const GLContext& glContext) const;

    private:
        ShaderType _type;
        String _source;
    };

public:
    ShaderPreprocessor(ShaderType type, const String& source);

    void addSource(const String& source);
    void addModifier(const String& modifier);

    void parse(PipelineBuildingContext& context, PipelineLayout& shader);

    Preprocessor preprocess();

    void insertPredefinedUniforms(const std::vector<Uniform>& uniforms);

private:
    void parseMainBlock(PipelineLayout& pipelineLayout);
    void parseDeclarations(PipelineBuildingContext& context);
    uint32_t parseFunctionBody(const String& s, String& body) const;

    String getDeclarations() const;
    void insertAfter(const String& statement, const String& str);

private:
    sp<CodeBlock> _main_block;

    friend class PipelineBuildingContext;
    friend class PipelineLayout;

public:
    ShaderType _type;
    String _source;

    StringBuffer _uniform_declarations;

    Declaration _in_declarations;
    Declaration _out_declarations;

    std::vector<Snippet> _snippets;
    std::vector<std::pair<String, String>> _uniforms;

};

class PipelineBuildingContext {
public:
    PipelineBuildingContext(PipelineLayout& pipelineLayout, const String& vertex, const String& fragment);

    void initialize();

    PipelineLayout& _pipeline_layout;

    ShaderPreprocessor _vertex;
    ShaderPreprocessor _fragment;

    std::map<String, Attribute> _attributes;

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

    Attribute& addPredefinedAttribute(const String& name, const String& type, uint32_t scopes);

private:
    Attribute makePredefinedAttribute(const String& name, const String& type);

private:
    void doSnippetPrecompile();
    void doPrecompile(String& vertSource, String& fragSource);

    void insertBefore(String& src, const String& statement, const String& str);

};

}

#endif
