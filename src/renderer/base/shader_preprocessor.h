#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/collection/table.h"

#include "renderer/forwarding.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/uniform.h"

namespace ark {

class ARK_API ShaderPreprocessor {
private:
    class Source;

public:
    struct Stage {
        document _manifest;
        enums::ShaderStageBit _type;
        String _source;
    };

    class Declaration {
    public:
        Declaration(String name, String type, uint32_t length, const PipelineLayout::Binding& binding, sp<String> source);
        DEFAULT_COPY_AND_ASSIGN(Declaration);

        bool operator <(const Declaration& other) const;

        void setSource(String source) const;

        String _name;
        String _type;
        uint32_t _length;
        PipelineLayout::Binding _binding;
        Attribute::Usage _usage;
        sp<String> _source;
    };

    class ARK_API DeclarationList {
    public:
        DeclarationList(Source& source, enums::ShaderTypeQualifier qualifier);

        bool has(const String& name) const;
        const Table<String, Declaration>& vars() const;
        Table<String, Declaration>& vars();

        void declare(const String& type, const char* prefix, const String& name, const String& layout = "", const char* qualifier = nullptr, bool isFlat = false);
        void clear();

    private:
        Source& _source;
        enums::ShaderTypeQualifier _qualifier;

        Table<String, Declaration> _vars;
    };

    struct SSBODeclaration {
        PipelineLayout::Binding _binding;
        sp<String> _declaration;
    };

private:
    class Source {
    public:
        Source() = default;
        Source(String code);

        String str(char endl = 0) const;

        void push_back(sp<String> fragment);

        bool search(const std::regex& pattern, const std::function<bool(const std::smatch& match)>& traveller) const;
        bool contains(const String& str) const;

        void replace(const String& str, const String& replacment);
        void replace(const std::regex& regexp, const std::function<sp<String>(const std::smatch&)>& replacer);

        void insertBefore(const String& statement, const String& str);

    private:
        List<sp<String>> _lines;
    };

    struct Parameter {
        enum Annotation {
            PARAMETER_ANNOTATION_DEFAULT = 0,
            PARAMETER_ANNOTATION_IN = 1,
            PARAMETER_ANNOTATION_OUT = 2,
            PARAMETER_ANNOTATION_INOUT = 3
        };
        Parameter();
        Parameter(String type, String name, Annotation annotation, uint32_t divisor);

        DEFAULT_COPY_AND_ASSIGN(Parameter);

        const char* getQualifierStr() const;

        String _type;
        String _name;
        Annotation _annotation;
        uint32_t _divisor;
    };

    struct Function {
        Function(String name, String params, String returnType, String body, sp<String> placeHolder);
        DEFAULT_COPY_AND_ASSIGN(Function);

        void parse(PipelineBuildingContext& buildingContext);

        void genDefinition();
        String genOutCall(enums::ShaderStageBit preShaderStage, enums::ShaderStageBit shaderStage) const;

        bool hasOutAttribute(const String& name) const;
        bool hasReturnValue() const;

        size_t outArgumentCount() const;

        String _name;
        String _params;
        String _return_type;
        String _body;
        Vector<Parameter> _args;
        sp<String> _place_hoder;

    private:
        static Parameter parseParameter(const String& param);
    };

public:
    ShaderPreprocessor(String source, document manifest, enums::ShaderStageBit shaderStage, enums::ShaderStageBit preShaderStage);

    void addPreMainSource(const String& source);
    void addPostMainSource(const String& source);
    void addOutputModifier(String preModifier, String postModifier);

    void initialize(PipelineBuildingContext& context);
    void initializeAsFirst(PipelineBuildingContext& context);

    void setupUniforms(const Table<String, sp<Uniform>>& uniforms);

    const char* inVarPrefix() const;
    const char* outVarPrefix() const;

    const Vector<Parameter>& args() const;

    void inDeclare(const String& type, const String& name);
    void outDeclare(const String& type, const String& name);
    void passThroughDeclare(const String& type, const String& name);

    void linkNextStage(const String& returnValueName);
    void linkPreStage(const ShaderPreprocessor& preStage, Set<String>& passThroughVars) const;

    Stage preprocess() const;

    sp<Uniform> makeUniform(String name, Uniform::Type type) const;

    bool hasUBO(const PipelineLayout::UBO& ubo) const;
    void declareUBOStruct(const PipelineLayout& pipelineLayout, int32_t spaceSet = -1);

    String outputName() const;

private:
    void parseMainBlock(const String& source, PipelineBuildingContext& buildingContext);
    void parseDeclarations();
    String genDeclarations(const String& mainFunc) const;

    void addInclude(const String& filepath);

    sp<String> addUniform(const String& type, const String& name, uint32_t length, const PipelineLayout::Binding& binding, String declaration);
    uint32_t getUniformSize(Uniform::Type type, const String& declaredType) const;
    uint32_t getUniformStructSize(const String& declaredType) const;

    void insertUBOStruct(const PipelineLayout::UBO& ubo, int32_t spaceSet);

    static void linkParameters(const Vector<Parameter>& parameters, const ShaderPreprocessor& preStage, Set<String>& passThroughVars);

private:
    String _source;
    sp<Function> _main_block;

    struct ResultModifer {
        String _pre_modifier;
        String _post_modifier;
    };

    friend class PipelineBuildingContext;
    friend class PipelineLayout;

public:
    document _manifest;
    enums::ShaderStageBit _shader_stage;
    enums::ShaderStageBit _pre_shader_stage;

    Source _main_source;

    Source _include_declaration_source;
    Source _struct_declaration_source;
    Source _uniform_declaration_source;
    Source _attribute_declaration_source;

    Table<String, String> _struct_definitions;

    int32_t _version;

    Vector<String> _predefined_macros;
    Vector<Parameter> _predefined_parameters;
    Vector<ResultModifer> _result_modifiers;

    Table<String, SSBODeclaration> _ssbos;

    Optional<V3i> _compute_local_sizes;

    DeclarationList _declaration_ins;
    DeclarationList _declaration_outs;
    DeclarationList _declaration_uniforms;
    DeclarationList _declaration_samplers;
    DeclarationList _declaration_images;

    sp<String> _pre_main;
    sp<String> _post_main;
};

}
