#pragma once

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/base/enum.h"
#include "core/collection/table.h"
#include "core/types/shared_ptr.h"
#include "core/types/optional.h"

#include "graphics/forwarding.h"

#include "renderer/base/pipeline_input.h"
#include "renderer/inf/model_loader.h"

namespace ark {

class ARK_API PipelineBindings {
public:
    enum Flag {
        FLAG_DYNAMIC_SCISSOR = 4,
        FLAG_DYNAMIC_SCISSOR_BITMASK = 4,
        FLAG_DEFAULT_VALUE = 0
    };

    enum TraitType {
        TRAIT_TYPE_CULL_FACE_TEST,
        TRAIT_TYPE_DEPTH_TEST,
        TRAIT_TYPE_STENCIL_TEST,
        TRAIT_TYPE_SCISSOR,
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
        BlendFactor _src_rgb_factor;
        BlendFactor _dst_rgb_factor;
        BlendFactor _src_alpha_factor;
        BlendFactor _dst_alpha_factor;
    };

    union TraitConfigure {
        TraitDepthTest _depth_test;
        TraitStencilTest _stencil_test;
        TraitCullFaceTest _cull_face_test;
        TraitBlend _blend;
    };

    struct ARK_API PipelineTraitMeta {
        PipelineTraitMeta(const document& manifest);
        PipelineTraitMeta(TraitType type, const TraitConfigure& configure);

        TraitType _type;
        TraitConfigure _configure;

    private:
        TraitStencilTestSeparate loadStencilTestSeparate(const document& manifest, bool allowDefaultFace) const;

    };

    typedef Table<TraitType, PipelineTraitMeta> PipelineTraitTable;

    struct ARK_API Parameters {
        Parameters(Optional<Rect> scissor, PipelineTraitTable tests, uint32_t flags);
        DEFAULT_COPY_AND_ASSIGN(Parameters);

        Optional<Rect> _scissor;
        PipelineTraitTable _traits;
        uint32_t _flags;

        class BUILDER {
        public:
            BUILDER(BeanFactory& factory, const document& manifest, const ResourceLoaderContext& resourceLoaderContext);

            Parameters build(const Scope& args) const;

        private:
            sp<RenderController> _render_controller;

            SafePtr<Builder<Vec4>> _pipeline_bindings_scissor;
            PipelineTraitTable _traits;
            [[deprecated]]
            uint32_t _pipeline_bindings_flags;
        };
    };

public:
    PipelineBindings(Enum::RenderMode mode, Enum::DrawProcedure renderProcedure, Parameters parameters, sp<PipelineLayout> pipelineLayout);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(PipelineBindings);

    Enum::RenderMode mode() const;
    Enum::DrawProcedure drawProcedure() const;

    const Parameters& parameters() const;
    const Optional<Rect>& scissor() const;
    const sp<PipelineLayout>& layout() const;
    const sp<PipelineInput>& input() const;

    const PipelineInput::AttributeOffsets& attributes() const;
    const Table<String, sp<Texture>>& samplers() const;
    const std::vector<sp<Texture>>& images() const;

    void bindSampler(sp<Texture> texture, uint32_t name = 0);

    bool hasDivisors() const;

    Flag getFlag(Flag bitmask) const;
    bool hasFlag(Flag flag, Flag bitmask) const;
    void setFlag(Flag flag, Flag bitmask) const;

    sp<Pipeline> getPipeline(GraphicsContext& graphicsContext, const sp<PipelineFactory>& pipelineFactory);

private:
    struct Stub {
        Stub(Enum::RenderMode mode, Enum::DrawProcedure renderProcedure, Parameters parameters, sp<PipelineLayout> pipelineLayout);

        Enum::RenderMode _mode;
        Enum::DrawProcedure _render_procedure;

        Parameters _parameters;

        sp<PipelineLayout> _layout;
        sp<PipelineInput> _input;
//TODO: move it to stream
        PipelineInput::AttributeOffsets _attributes;

        Table<String, sp<Texture>> _samplers;
        std::vector<sp<Texture>> _images;
    };

private:
    sp<Stub> _stub;

    sp<Pipeline> _pipeline;
};

}
