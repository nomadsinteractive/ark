#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/collection/table.h"
#include "core/types/owned_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/shader_preprocessor.h"
#include "renderer/base/texture.h"
#include "renderer/base/uniform.h"

namespace ark {

class ARK_API PipelineBuildingContext {
public:
    PipelineBuildingContext();
    PipelineBuildingContext(sp<String> vertex, sp<String> fragment);

    DISALLOW_COPY_AND_ASSIGN(PipelineBuildingContext);

    void loadManifest(const document& manifest, BeanFactory& factory, const Scope& args);

    void initialize(const Camera& camera);

    enum LayoutBindingType {
        LAYOUT_BINDING_TYPE_AUTO,
        LAYOUT_BINDING_TYPE_IMAGE,
        LAYOUT_BINDING_TYPE_SAMPLER,
        LAYOUT_BINDING_TYPE_SSBO,
        LAYOUT_BINDING_TYPE_UBO
    };

    struct LayoutBinding {
        LayoutBindingType _type;
        sp<Texture> _texture;
        Texture::Usage _usage;
        String _name;
        int32_t _binding;
    };

    sp<PipelineLayout> _pipeline_layout;

    Map<String, Attribute> _attributes;
    Map<String, sp<StringVar>> _definitions;
    Vector<LayoutBinding> _layout_bindings;
    Table<String, sp<Uniform>> _uniforms;
    Table<String, sp<Texture>> _samplers;
    Table<String, sp<Texture>> _images;
    Table<String, Buffer> _ssbos;
    Map<HashId, sp<PipelineLayout::UBO>> _ubos;

    Table<String, std::pair<String, uint32_t>> _input_attributes;

    const Vector<ShaderPreprocessor*>& stages() const;
    const Map<Enum::ShaderStageBit, op<ShaderPreprocessor>>& renderStages() const;
    const op<ShaderPreprocessor>& computingStage() const;

    void addAttribute(String name, String type, uint32_t divisor);
    void addUniform(String name, Uniform::Type type, uint32_t length, sp<Uploader> input);
    void addUniform(sp<Uniform> uniform);

    void addInputAttribute(const String& name, const String& type, uint32_t divisor);
    Attribute& addPredefinedAttribute(const String& name, const String& type, uint32_t divisor,Enum::ShaderStageBit stage);

    ShaderPreprocessor* tryGetRenderStage(Enum::ShaderStageBit shaderStage) const;
    const op<ShaderPreprocessor>& getRenderStage(Enum::ShaderStageBit shaderStage) const;
    const op<ShaderPreprocessor>& addStage(sp<String> source, document manifest, Enum::ShaderStageBit shaderStage, Enum::ShaderStageBit preShaderStage);

    Map<String, String> toDefinitions() const;

    void tryBindCamera(const ShaderPreprocessor& shaderPreprocessor, const Camera& camera);

private:
    void initializeAttributes();
    void initializeStages(const Camera& camera);
    void initializeSSBO() const;
    void initializeUniforms();

    void tryBindUniformMatrix(const ShaderPreprocessor& shaderPreprocessor, String name, const sp<Mat4>& matrix);

    void loadPredefinedAttribute(const document& manifest);
    void loadPredefinedUniform(BeanFactory& factory, const Scope& args, const document& manifest);
    void loadPredefinedSampler(BeanFactory& factory, const Scope& args, const document& manifest);
    void loadPredefinedImage(BeanFactory& factory, const Scope& args, const document& manifest);
    void loadPredefinedBuffer(BeanFactory& factory, const Scope& args, const document& manifest);
    void loadLayoutBindings(BeanFactory& factory, const Scope& args, const document& manifest);
    void loadDefinitions(BeanFactory& factory, const Scope& args, const document& manifest);

    Map<Enum::ShaderStageBit, op<ShaderPreprocessor>> _rendering_stages;
    op<ShaderPreprocessor> _computing_stage;
    Vector<ShaderPreprocessor*> _stages;
};

}
