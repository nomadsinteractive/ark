#ifndef ARK_RENDERER_BASE_SHADER_PREPROCESSOR_H_
#define ARK_RENDERER_BASE_SHADER_PREPROCESSOR_H_

#include <map>

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/base/string_buffer.h"
#include "core/collection/table.h"
#include "core/types/global.h"

#include "renderer/forwarding.h"
#include "renderer/base/attribute.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/uniform.h"

namespace ark {

class ShaderPreprocessor {
private:
    class Source;

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

    class Preprocessor {
    public:
        Preprocessor();
        Preprocessor(ShaderType type, String source);
        DEFAULT_COPY_AND_ASSIGN(Preprocessor);

        ShaderType type() const;

        String process(const RenderEngineContext& glContext) const;

    private:
        ShaderType _type;
        String _source;
    };

    class Declaration {
    public:
        Declaration(const String& name, const String& type, uint32_t length, const sp<String>& source);
        DEFAULT_COPY_AND_ASSIGN(Declaration);

        const String& name() const;
        const String& type() const;
        uint32_t length() const;

        const sp<String>& source() const;

    private:
        String _name;
        String _type;
        uint32_t _length;

        sp<String> _source;
    };

    class DeclarationList {
    public:
        DeclarationList(Source& source, const String& descriptor);

        bool has(const String& name) const;
        const Table<String, Declaration>& vars() const;
        Table<String, Declaration>& vars();

        void declare(const String& type, const String& prefix, const String& name);
        void parse(const std::regex& pattern);

    private:
        Source& _source;
        String _descriptor;

        Table<String, Declaration> _vars;
    };

private:
    class Source {
    public:
        Source() = default;
        Source(String code);

        String str(char endl = 0) const;

        void push_front(const sp<String>& fragment);
        void push_back(const sp<String>& fragment);

        bool search(const std::regex& pattern, const std::function<bool(const std::smatch& match)>& traveller) const;
        bool contains(const String& str) const;

        void replace(const String& str, const String& replacment);
        void replace(const std::regex& regexp, const std::function<sp<String>(const std::smatch&)>& replacer);

        void insertBefore(const String& statement, const String& str);

    private:
        std::list<sp<String>> _fragments;
    };

    struct Parameter {
        enum Modifier {
            PARAMETER_MODIFIER_DEFAULT = 0,
            PARAMETER_MODIFIER_IN = 1,
            PARAMETER_MODIFIER_OUT = 2,
            PARAMETER_MODIFIER_IN_OUT = 3
        };
        Parameter();
        Parameter(String type, String name, Modifier modifier);

        DEFAULT_COPY_AND_ASSIGN(Parameter);

        String _type;
        String _name;
        Modifier _modifier;
    };

    struct Function {
        Function(const String& name, const String& params, const String& body);
        DEFAULT_COPY_AND_ASSIGN(Function);

        String _name;
        String _params;
        String _body;
//        std::vector<std::pair<String, String>> _ins;
//        std::vector<std::pair<String, String>> _outs;
        std::vector<Parameter> _ins;
        std::vector<Parameter> _outs;
    };

    struct CodeBlock {
        CodeBlock(const Function& procedure, const sp<String>& placeHolder);
        DEFAULT_COPY_AND_ASSIGN(CodeBlock);

        void parse(PipelineBuildingContext& buildingContext);
        Parameter parseParameter(const String& param);

        void genDefinition();
        String genOutCall(ShaderType type);

        bool hasOutParam(const String& name) const;

        Function _function;
        sp<String> _place_hoder;
    };

public:
    ShaderPreprocessor(ShaderType type);

    void addPreMainSource(const String& source);
    void addPostMainSource(const String& source);
    void addModifier(const String& modifier);

    void initialize(const String& source, PipelineBuildingContext& context);

    void setupUniforms(Table<String, sp<Uniform>>& uniforms, int32_t& counter);

    Preprocessor preprocess();

    sp<Uniform> getUniformInput(const String& name, Uniform::Type type) const;

    String outputName() const;

private:
    void parseMainBlock(const String& source, PipelineBuildingContext& buildingContext);
    void parseDeclarations(PipelineBuildingContext& context);
    size_t parseFunctionBody(const String& s, String& body) const;

    String genDeclarations() const;

    void addInclude(const String& source, const String& filepath);
    void addUniform(const String& type, const String& name, uint32_t length, const sp<String>& declaration);
    uint32_t getUniformSize(Uniform::Type type, const String& declaredType) const;

private:
    sp<CodeBlock> _main_block;

    friend class PipelineBuildingContext;
    friend class PipelineLayout;

public:
    ShaderType _type;

    Source _main;

    Source _includes;
    Source _struct_declarations;
    Table<String, String> _struct_definitions;
    Source _uniform_declarations;
    Source _attribute_declarations;

    int32_t _version;

    std::vector<String> _macro_defines;

    DeclarationList _ins;
    DeclarationList _outs;

    DeclarationList _uniforms;
    DeclarationList _samplers;

    sp<String> _pre_main;
    sp<String> _output_var;
    sp<String> _post_main;
};

}

#endif
