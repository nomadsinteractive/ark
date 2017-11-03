#ifndef ARK_RENDERER_UTIL_GL_SHADER_PREPROCESSOR_H_
#define ARK_RENDERER_UTIL_GL_SHADER_PREPROCESSOR_H_

#include <map>

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/base/string_builder.h"
#include "core/collection/list.h"
#include "core/types/global.h"

#include "renderer/forwarding.h"
#include "renderer/base/render_engine.h"

namespace ark {

class GLShaderPreprocessor {
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
        Snippet(const Snippet& other);
        Snippet(Snippet&& other);

        SnippetType _type;
        String _src;
    };

    struct Procedure {
        String _name;
        String _params;
        String _body;
        List<std::pair<String, String>> _ins;
        List<std::pair<String, String>> _outs;

        Procedure(const String& name, const String& params, const String& body);
        Procedure(const Procedure& other);
        Procedure(Procedure&& other);
    };

    struct CodeBlock {
        String _prefix;
        Procedure _procedure;
        String _suffix;

        CodeBlock(const String& prefix, const Procedure& procedure, const String& suffix);
        CodeBlock(const CodeBlock& other);
        CodeBlock(CodeBlock&& other);
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

        StringBuilder _lines;
        std::map<String, String> _declared;
    };

public:
    enum ShaderType {
        SHADER_TYPE_VERTEX,
        SHADER_TYPE_FRAGMENT
    };

    struct Context {
        List<std::pair<String, String>> vertexIns;
        List<std::pair<String, String>> vertexOuts;
        List<std::pair<String, String>> fragmentIns;

        std::map<String, String> vertexInsDeclared;

        List<Snippet> _vertex_snippets;
        List<Snippet> _fragment_snippets;

        void addAttribute(const String& name, const String& type, std::map<String, String>& vars, GLShaderSource& source);
        void addVertexSource(const String& source);
        void addFragmentColorModifier(const String& modifier);
        void addFragmentProcedure(const String& name, const List<std::pair<String, String>>& ins, const String& procedure);
    };

    static const char* ANNOTATION_VERT_IN;
    static const char* ANNOTATION_VERT_OUT;
    static const char* ANNOTATION_FRAG_IN;
    static const char* ANNOTATION_FRAG_OUT;
    static const char* ANNOTATION_FRAG_COLOR;

    static std::regex _IN_PATTERN;
    static std::regex _OUT_PATTERN;
    static std::regex _IN_OUT_PATTERN;

public:
    GLShaderPreprocessor(ShaderType type, const String& source);

    void parse1(GLShaderSource& shader);
    void parse2(Context& context, GLShaderSource& shader);
    void preprocess();

    void addUniform(const String& type, const String& name);

    String process(const GLContext& glContext) const;

    void insertPredefinedUniforms(const List<GLUniform>& uniforms);

private:
    void parseCodeBlock(CodeBlock& codeBlock, GLShaderSource& shader);
    uint32_t parseFunctionBody(const String& s, String& body);

    String getDeclarations();
    void insertAfter(const String& statement, const String& str);

private:
    sp<CodeBlock> _main_block;

public:
    ShaderType _type;
    String _source;

    StringBuilder _uniform_declarations;
    std::map<String, String> _annotations;

public:
    Declaration _in_declarations;
    Declaration _out_declarations;


    void declare(StringBuilder& sb, const List<std::pair<String, String>>& vars, const String& inType, const String& prefix, std::map<String, String>& declared) const;

    friend class GLShaderSource;
};

}

#endif
