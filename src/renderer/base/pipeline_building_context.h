#pragma once

#include <map>
#include <set>

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
    PipelineBuildingContext(const sp<RenderController>& renderController, const sp<Camera>& camera);
    PipelineBuildingContext(const sp<RenderController>& renderController, const sp<Camera>& camera, sp<String> vertex, sp<String> fragment);

    DISALLOW_COPY_AND_ASSIGN(PipelineBuildingContext);

    void loadManifest(const document& manifest, BeanFactory& factory, const Scope& args);

    void initialize();

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

    sp<RenderController> _render_controller;
    sp<PipelineInput> _input;
    sp<Snippet> _snippet;

    std::map<String, Attribute> _attributes;
    std::map<String, sp<StringVar>> _definitions;
    std::vector<LayoutBinding> _layout_bindings;
    Table<String, sp<Uniform>> _uniforms;
    Table<String, sp<Texture>> _samplers;
    Table<String, sp<Texture>> _images;
    Table<String, Buffer> _ssbos;
    std::map<HashId, sp<PipelineInput::UBO>> _ubos;

    std::set<String> _input_vars;

    const std::map<ShaderStage::BitSet, op<ShaderPreprocessor>>& stages() const;

    void addAttribute(String name, String type, uint32_t divisor);
    void addSnippet(const sp<Snippet>& snippet);
    void addUniform(String name, Uniform::Type type, uint32_t length, sp<Uploader> input);
    void addUniform(sp<Uniform> uniform);

    void addInputAttribute(const String& name, const String& type, uint32_t divisor);
    Attribute& addPredefinedAttribute(const String& name, const String& type, uint32_t divisor,ShaderStage::BitSet stage);

    bool hasStage(ShaderStage::BitSet shaderStage) const;
    ShaderPreprocessor* tryGetStage(ShaderStage::BitSet shaderStage) const;
    const op<ShaderPreprocessor>& getStage(ShaderStage::BitSet shaderStage) const;
    const op<ShaderPreprocessor>& addStage(sp<String> source, ShaderStage::BitSet shaderStage, ShaderStage::BitSet preShaderStage);

    sp<Snippet> makePipelineSnippet();
    std::map<String, String> toDefinitions() const;

    void tryBindCamera(const ShaderPreprocessor& shaderPreprocessor);

private:
    void initializeAttributes();
    void initializeStages();
    void initializeSSBO();
    void initializeUniforms();

    void tryBindUniformMatrix(const ShaderPreprocessor& shaderPreprocessor, String name, sp<Mat4> matrix);

    void loadPredefinedAttribute(const document& manifest);
    void loadPredefinedUniform(BeanFactory& factory, const Scope& args, const document& manifest);
    void loadPredefinedSampler(BeanFactory& factory, const Scope& args, const document& manifest);
    void loadPredefinedImage(BeanFactory& factory, const Scope& args, const document& manifest);
    void loadPredefinedBuffer(BeanFactory& factory, const Scope& args, const document& manifest);
    void loadLayoutBindings(BeanFactory& factory, const Scope& args, const document& manifest);
    void loadDefinitions(BeanFactory& factory, const Scope& args, const document& manifest);

    std::map<ShaderStage::BitSet, op<ShaderPreprocessor>> _stages;
};

}
