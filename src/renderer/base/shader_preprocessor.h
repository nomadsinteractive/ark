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
#include "renderer/base/pipeline_input.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/shader.h"
#include "renderer/base/uniform.h"

namespace ark {

class ShaderPreprocessor {
private:
    class Source;

public:
    static const char* ANNOTATION_VERT_IN;
    static const char* ANNOTATION_VERT_OUT;
    static const char* ANNOTATION_FRAG_IN;
    static const char* ANNOTATION_FRAG_OUT;
    static const char* ANNOTATION_FRAG_COLOR;

    class Preprocessed {
    public:
        Preprocessed();
        Preprocessed(PipelineInput::ShaderStage stage, String source);
        DEFAULT_COPY_AND_ASSIGN(Preprocessed);

        PipelineInput::ShaderStage stage() const;

        String toSourceCode(const RenderEngineContext& renderEngineContext) const;

    private:
        PipelineInput::ShaderStage _type;
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

        void declare(const String& type, const char* prefix, const String& name, int32_t location, const char* qualifier = nullptr);

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
            PARAMETER_MODIFIER_INOUT = 3
        };
        Parameter();
        Parameter(String type, String name, Modifier modifier);

        DEFAULT_COPY_AND_ASSIGN(Parameter);

        const char* getQualifierStr() const;

        String _type;
        String _name;
        Modifier _modifier;
    };

    struct Function {
        Function(String name, String params, String returnType, String body, sp<String> placeHolder);
        DEFAULT_COPY_AND_ASSIGN(Function);

        void parse(PipelineBuildingContext& buildingContext);

        void genDefinition();
        String genOutCall(PipelineInput::ShaderStage preShaderStage, PipelineInput::ShaderStage shaderStage) const;

        bool hasOutAttribute(const String& name) const;
        bool hasReturnValue() const;

        String _name;
        String _params;
        String _return_type;
        String _body;
        std::vector<Parameter> _ins;
        std::vector<Parameter> _outs;

        sp<String> _place_hoder;

    private:
        Parameter parseParameter(const String& param);

    };

public:
    ShaderPreprocessor(sp<String> source, PipelineInput::ShaderStage shaderStage, PipelineInput::ShaderStage preShaderStage);

    void addPreMainSource(const String& source);
    void addPostMainSource(const String& source);
    void addOutputVarModifier(String modifier);

    void initialize(PipelineBuildingContext& context);
    void initializeAsFirst(PipelineBuildingContext& context);

    void setupUniforms(Table<String, sp<Uniform>>& uniforms, int32_t& binding);

    const char* inVarPrefix() const;
    const char* outVarPrefix() const;

    void inDeclare(const String& type, const String& name, int32_t location = -1);
    void outDeclare(const String& type, const String& name, int32_t location = -1);
    void linkNextStage(const String& returnValueName);

    void linkPreStage(const ShaderPreprocessor& preStage, std::set<String>& passThroughVars);

    Preprocessed preprocess();

    sp<Uniform> getUniformInput(const String& name, Uniform::Type type) const;

    String outputName() const;

private:
    void parseMainBlock(const String& source, PipelineBuildingContext& buildingContext);
    void parseDeclarations();
    size_t parseFunctionBody(const String& s, String& body) const;

    String genDeclarations(const String& mainFunc) const;

    void addInclude(const String& source, const String& filepath);
    void addUniform(const String& type, const String& name, uint32_t length, const sp<String>& declaration);
    uint32_t getUniformSize(Uniform::Type type, const String& declaredType) const;

    void linkParameters(const std::vector<Parameter>& parameters, const ShaderPreprocessor& preStage, std::set<String>& passThroughVars);

    static const char* getOutAttributePrefix(PipelineInput::ShaderStage preStage);

private:
    sp<String> _source;
    sp<Function> _main_block;

    friend class PipelineBuildingContext;
    friend class PipelineLayout;

public:
    PipelineInput::ShaderStage _shader_stage;
    PipelineInput::ShaderStage _pre_shader_stage;

    Source _main;

    Source _includes;
    Source _struct_declarations;
    Source _uniform_declarations;
    Source _attribute_declarations;

    Table<String, String> _struct_definitions;

    int32_t _version;

    std::vector<String> _predefined_macros;
    std::vector<Parameter> _predefined_parameters;
    std::vector<String> _output_var_modifiers;

    std::map<std::string, int32_t> _ssbos;

    DeclarationList _declaration_ins;
    DeclarationList _declaration_outs;
    DeclarationList _declaration_uniforms;
    DeclarationList _declaration_samplers;

    sp<String> _pre_main;
    sp<String> _post_main;
};

}

#endif
