#pragma once

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/base/enum.h"
#include "core/collection/table.h"
#include "core/types/shared_ptr.h"
#include "core/types/optional.h"

#include "graphics/forwarding.h"

#include "renderer/base/shader_layout.h"
#include "renderer/inf/model_loader.h"

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
    };

    union TraitConfigure {
        TraitDepthTest _depth_test;
        TraitStencilTest _stencil_test;
        TraitCullFaceTest _cull_face_test;
        TraitScissorTest _scissor_test;
        TraitBlend _blend;
    };

    typedef Table<TraitType, TraitConfigure> PipelineTraitTable;

    struct Parameters {
        Optional<Rect> _scissor;
        PipelineTraitTable _traits;

        class BUILDER {
        public:
            BUILDER(BeanFactory& factory, const document& manifest);

            Parameters build(const Scope& args) const;

        private:
            SafeBuilder<Vec4> _scissor;
            PipelineTraitTable _traits;
        };
    };

public:
    PipelineDescriptor(Enum::RenderMode mode, Enum::DrawProcedure drawProcedure, Parameters parameters, sp<PipelineConfiguration> configuration);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(PipelineDescriptor);

    Enum::RenderMode mode() const;
    Enum::DrawProcedure drawProcedure() const;

    const Parameters& parameters() const;
    const Optional<Rect>& scissor() const;
    const sp<PipelineConfiguration>& configuration() const;
    const sp<ShaderLayout>& shaderLayout() const;

    const ShaderLayout::AttributeOffsets& attributes() const;

    const Vector<std::pair<sp<Texture>, ShaderLayout::DescriptorSet>>& samplers() const;
    const Vector<std::pair<sp<Texture>, ShaderLayout::DescriptorSet>>& images() const;

    void bindSampler(sp<Texture> texture, uint32_t name = 0);

    bool hasDivisors() const;
    bool hasTrait(TraitType traitType) const;

private:
    Enum::RenderMode _mode;
    Enum::DrawProcedure _render_procedure;

    Parameters _parameters;

    sp<PipelineConfiguration> _configuration;
    sp<ShaderLayout> _shader_layout;
    //TODO: move it to stream
    ShaderLayout::AttributeOffsets _attributes;

    Vector<std::pair<sp<Texture>, ShaderLayout::DescriptorSet>> _samplers;
    Vector<std::pair<sp<Texture>, ShaderLayout::DescriptorSet>> _images;
};

}
