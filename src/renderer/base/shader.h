#pragma once

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/render_layer.h"

#include "renderer/forwarding.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/inf/model_loader.h"
#include "renderer/inf/resource.h"
#include "renderer/inf/pipeline_factory.h"

namespace ark {

class ARK_API Shader {
public:
    typedef std::map<PipelineInput::ShaderStage, sp<Builder<String>>> StageManifest;
    typedef std::vector<sp<Builder<Snippet>>> SnippetManifest;

public:
    Shader(sp<PipelineFactory> pipelineFactory, sp<RenderController> renderController, sp<PipelineLayout> layout, PipelineBindings::Parameters bindingParams);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Shader);

    static sp<Builder<Shader>> fromDocument(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& defVertex = "shaders/default.vert", const String& defFragment = "shaders/texture.frag", const sp<Camera>& defaultCamera = nullptr);

    std::vector<RenderLayerSnapshot::UBOSnapshot> takeUBOSnapshot(const RenderRequest& renderRequest) const;
    std::vector<std::pair<uint32_t, Buffer::Snapshot>> takeSSBOSnapshot(const RenderRequest& renderRequest) const;

    const sp<PipelineFactory>& pipelineFactory() const;
    const sp<RenderController>& renderController() const;

//  [[script::bindings::property]]
    const sp<Camera>& camera() const;
    const sp<PipelineInput>& input() const;
    const sp<PipelineLayout>& layout() const;

    sp<ShaderBindings> makeBindings(Buffer vertices, ModelLoader::RenderMode mode, PipelineBindings::DrawProcedure renderProcedure, const std::map<uint32_t, sp<Uploader>>& uploaders = {}) const;

    class BUILDER_IMPL : public Builder<Shader> {
    public:
        BUILDER_IMPL(BeanFactory& factory, const document& manifest, const ResourceLoaderContext& resourceLoaderContext, sp<Builder<Camera>> camera = nullptr,
                     Optional<StageManifest> stages = Optional<StageManifest>(), Optional<SnippetManifest> snippets = Optional<SnippetManifest>());

        virtual sp<Shader> build(const Scope& args) override;

    private:
        StageManifest loadStages(BeanFactory& factory, const document& manifest) const;

        sp<PipelineBuildingContext> makePipelineBuildingContext(const Scope& args) const;

    private:
        BeanFactory _factory;
        document _manifest;
        sp<RenderController> _render_controller;

        StageManifest _stages;
        SnippetManifest _snippets;
        SafePtr<Builder<Camera>> _camera;
        PipelineBindings::Parameters::BUILDER _parameters;
    };

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<Shader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Shader> build(const Scope& args) override;

    private:
        BUILDER_IMPL _impl;
    };

private:
    std::map<uint32_t, Buffer> makeDivivedBuffers(const std::map<uint32_t, sp<Uploader>>& uploaders) const;

private:
    sp<PipelineFactory> _pipeline_factory;
    sp<RenderController> _render_controller;
    sp<PipelineLayout> _pipeline_layout;
    sp<PipelineInput> _pipeline_input;

    PipelineBindings::Parameters _binding_params;
};

}
