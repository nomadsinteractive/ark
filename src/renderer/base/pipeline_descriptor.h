#pragma once

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/base/enum.h"
#include "core/collection/table.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/shader_preprocessor.h"

namespace ark {

class ARK_API PipelineDescriptor {
public:
    enum TraitType {
        TRAIT_TYPE_CULL_FACE_TEST,
        TRAIT_TYPE_DEPTH_TEST,
        TRAIT_TYPE_STENCIL_TEST,
        TRAIT_TYPE_SCISSOR_TEST,
        TRAIT_TYPE_BLEND
    };

    enum CompareFunc {
        COMPARE_FUNC_DEFAULT,
        COMPARE_FUNC_ALWAYS,
        COMPARE_FUNC_NEVER,
        COMPARE_FUNC_EQUAL,
        COMPARE_FUNC_NOT_EQUAL,
        COMPARE_FUNC_LESS,
        COMPARE_FUNC_GREATER,
        COMPARE_FUNC_LEQUAL,
        COMPARE_FUNC_GEQUAL,
        COMPARE_FUNC_LENGTH
    };

    enum StencilFunc {
        STENCIL_FUNC_KEEP,
        STENCIL_FUNC_ZERO,
        STENCIL_FUNC_REPLACE,
        STENCIL_FUNC_INCREASE,
        STENCIL_FUNC_INCREASE_AND_WRAP,
        STENCIL_FUNC_DECREASE,
        STENCIL_FUNC_DECREASE_AND_WRAP,
        STENCIL_FUNC_INVERT,
        STENCIL_FUNC_LENGTH
    };

    enum FrontFace {
        FRONT_FACE_DEFAULT,
        FRONT_FACE_COUTER_CLOCK_WISE,
        FRONT_FACE_CLOCK_WISE,
        FRONT_FACE_LENGTH
    };

    enum FrontFaceType {
        FRONT_FACE_TYPE_DEFAULT,
        FRONT_FACE_TYPE_FRONT,
        FRONT_FACE_TYPE_BACK,
        FRONT_FACE_TYPE_LENGTH
    };

    enum BlendFactor {
        BLEND_FACTOR_DEFAULT,
        BLEND_FACTOR_ZERO,
        BLEND_FACTOR_ONE,
        BLEND_FACTOR_SRC_COLOR,
        BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
        BLEND_FACTOR_DST_COLOR,
        BLEND_FACTOR_ONE_MINUS_DST_COLOR,
        BLEND_FACTOR_SRC_ALPHA,
        BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        BLEND_FACTOR_DST_ALPHA,
        BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
        BLEND_FACTOR_CONST_COLOR,
        BLEND_FACTOR_CONST_ALPHA,
    };

    struct TraitDepthTest {
        bool _enabled;
        bool _write_enabled;
        CompareFunc _func;
    };

    struct TraitStencilTestSeparate {
        FrontFaceType _type;
        CompareFunc _func;
        uint32_t _mask;
        uint32_t _compare_mask;
        uint32_t _ref;
        StencilFunc _op, _op_dfail, _op_dpass;
    };

    struct TraitStencilTest {
        TraitStencilTestSeparate _front;
        TraitStencilTestSeparate _back;
    };

    struct TraitCullFaceTest {
        bool _enabled;
        FrontFace _front_face;
    };

    struct TraitBlend {
        bool _enabled;
        BlendFactor _src_rgb_factor;
        BlendFactor _dst_rgb_factor;
        BlendFactor _src_alpha_factor;
        BlendFactor _dst_alpha_factor;
    };

    struct TraitScissorTest {
        bool _dynamic;
    };

    typedef std::variant<TraitDepthTest, TraitStencilTest, TraitCullFaceTest, TraitScissorTest, TraitBlend> Trait;
    typedef Vector<Trait> PipelineTraitTable;

    struct Configuration {
        PipelineTraitTable _traits;
        sp<Vec4> _scissor;
        sp<Vec3i> _num_work_groups;
        sp<Snippet> _snippet;
        sp<DrawDecoratorFactory> _draw_decorator_factory;

        class BUILDER {
        public:
            BUILDER(BeanFactory& factory, const document& manifest);

            Configuration build(const Scope& args) const;

        private:
            SafeBuilder<Vec4> _scissor;
            SafeBuilder<Vec3i> _num_work_groups;
            PipelineTraitTable _traits;
        };
    };

    struct BindedTexture {
        String _name;
        sp<Texture> _texture;
        PipelineLayout::DescriptorSet _descriptor_set;
    };

public:
    PipelineDescriptor(Camera camera, PipelineBuildingContext& buildingContext, Configuration configuration);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(PipelineDescriptor);

    const Configuration& configuration() const;
    void setConfiguration(Configuration configuration);

    const Camera& camera() const;
    Camera& camera();

    const sp<Vec4>& scissor() const;

    const sp<Snippet>& snippet() const;
    const sp<PipelineLayout>& layout() const;

    const PipelineLayout::VertexDescriptor& vertexDescriptor() const;

    Vector<BindedTexture> makeBindingSamplers() const;
    Vector<BindedTexture> makeBindingImages() const;

    Map<enums::ShaderStageBit, ShaderPreprocessor::Stage> getPreprocessedStages(const RenderEngineContext& renderEngineContext) const;

    bool hasDivisors() const;
    const String& signature() const;

    template<typename T> const T* getTrait() const {
        for(const Trait& i : _configuration._traits)
            if(const T* ptr = std::get_if<T>(&i))
                return ptr;
        return nullptr;
    }

private:
    String generateSignature() const;

    Camera _camera;
    Configuration _configuration;

    sp<PipelineLayout> _layout;

    Table<String, sp<Texture>> _predefined_samplers;
    Table<String, sp<Texture>> _predefined_images;

    Map<String, String> _definitions;

    Vector<ShaderPreprocessor::Stage> _stages;
    String _signature;
};

}
