#ifndef ARK_RENDERER_BASE_PIPELINE_BINDINGS_H_
#define ARK_RENDERER_BASE_PIPELINE_BINDINGS_H_

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/collection/table.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/base/pipeline_input.h"
#include "renderer/inf/model_loader.h"

namespace ark {

class ARK_API PipelineBindings {
public:
    enum Flag {
        FLAG_CULL_MODE_CW = 1,
        FLAG_CULL_MODE_CCW = 2,
        FLAG_CULL_MODE_NONE = 3,
        FLAG_CULL_MODE_BITMASK = 3,
        FLAG_DYNAMIC_SCISSOR = 4,
        FLAG_DYNAMIC_SCISSOR_BITMASK = 4,
        FLAG_DEFAULT_VALUE = FLAG_CULL_MODE_CCW
    };

    enum RenderProcedure {
        RENDER_PROCEDURE_DRAW_ARRAYS,
        RENDER_PROCEDURE_DRAW_ELEMENTS,
        RENDER_PROCEDURE_DRAW_ELEMENTS_INSTANCED,
        RENDER_PROCEDURE_DRAW_MULTI_ELEMENTS_INDIRECT
    };

    enum FragmentTest {
        FRAGMENT_TEST_CULL_FACE,
        FRAGMENT_TEST_DEPTH,
        FRAGMENT_TEST_SCISSOR,
        FRAGMENT_TEST_STENCIL
    };

    enum CompareFunc {
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

    enum FrontFace {
        FRONT_FACE_COUTER_CLOCK_WISE,
        FRONT_FACE_CLOCK_WISE,
        FRONT_FACE_LENGTH
    };

    struct TraitDepthTest {
        bool _enabled;
        bool _write_enabled;
        CompareFunc _func;
    };

    struct TraitCullFaceTest {
        bool _enabled;
        FrontFace _front_face;
    };

    union Trait {
        TraitDepthTest _depth_test;
        TraitCullFaceTest _cull_face_test;
    };

    struct FragmentTestManifest {
        FragmentTestManifest(document manifest, sp<Vec4> value = nullptr);

        document _manifest;
        sp<Vec4> _value;

        FragmentTest _type;
        Trait _trait;

    };

    typedef Table<int32_t, FragmentTestManifest> FragmentTestTable;

    struct ARK_API Parameters {
        Parameters(const Rect& scissor, FragmentTestTable tests, uint32_t flags);
        DEFAULT_COPY_AND_ASSIGN(Parameters);

        Rect _scissor;
        FragmentTestTable _tests;
        uint32_t _flags;

        class BUILDER {
        public:
            BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

            Parameters build(const Scope& args) const;

        private:
            sp<RenderController> _render_controller;

            SafePtr<Builder<Vec4>> _pipeline_bindings_scissor;
            FragmentTestTable _tests;
            uint32_t _pipeline_bindings_flags;
        };
    };

public:
    PipelineBindings(ModelLoader::RenderMode mode, RenderProcedure renderProcedure, Parameters parameters, sp<PipelineLayout> pipelineLayout);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(PipelineBindings);

    ModelLoader::RenderMode mode() const;
    RenderProcedure renderProcedure() const;
    const Rect& scissor() const;
    const Parameters& parameters() const;

    const sp<PipelineLayout>& layout() const;
    const sp<PipelineInput>& input() const;
    const sp<Snippet>& snippet() const;

    const PipelineInput::Attributes& attributes() const;
    const std::vector<sp<Texture>>& samplers() const;

    void bindSampler(const sp<Texture>& texture, uint32_t name = 0);

    bool hasDivisors() const;

    Flag getFlag(Flag bitmask) const;
    bool hasFlag(Flag flag, Flag bitmask) const;
    void setFlag(Flag flag, Flag bitmask) const;

    sp<Pipeline> getPipeline(GraphicsContext& graphicsContext, const sp<PipelineFactory>& pipelineFactory);

private:
    struct Stub {
        Stub(ModelLoader::RenderMode mode, RenderProcedure renderProcedure, Parameters parameters, sp<PipelineLayout> pipelineLayout);

        ModelLoader::RenderMode _mode;
        RenderProcedure _render_procedure;

        Parameters _parameters;

        sp<PipelineLayout> _layout;
        sp<PipelineInput> _input;

        PipelineInput::Attributes _attributes;

        std::vector<sp<Texture>> _samplers;
    };

private:
    sp<Stub> _stub;

    sp<Pipeline> _pipeline;
};

}

#endif
