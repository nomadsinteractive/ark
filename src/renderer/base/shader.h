#pragma once

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/inf/pipeline_factory.h"

namespace ark {

//[[script::bindings::auto]]
class ARK_API Shader {
public:
    struct StageManifest {
        StageManifest(Enum::ShaderStageBit type, builder<String> source);
        StageManifest(BeanFactory& factory, const document& manifest);

        Enum::ShaderStageBit _type;
        builder<String> _source;
        document _manifest;
    };

    typedef std::vector<sp<Builder<Snippet>>> SnippetManifest;

public:
    Shader(sp<PipelineFactory> pipelineFactory, sp<RenderController> renderController, sp<PipelineLayout> layout, PipelineDescriptor::Parameters bindingParams);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Shader);

    static sp<Builder<Shader>> fromDocument(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& defVertex = "shaders/default.vert", const String& defFragment = "shaders/texture.frag", const sp<Camera>& defaultCamera = nullptr);

    [[deprecated]]
    sp<RenderLayerSnapshot::BufferObject> takeBufferSnapshot(const RenderRequest& renderRequest, bool isComputeStage) const;

    const sp<PipelineFactory>& pipelineFactory() const;
    const sp<RenderController>& renderController() const;

    void setCamera(const Camera& camera);
    const sp<ShaderLayout>& input() const;
    const sp<PipelineLayout>& layout() const;

    const PipelineDescriptor::Parameters& descriptorParams() const;

    sp<PipelineBindings> makeBindings(Buffer vertices, Enum::RenderMode mode, Enum::DrawProcedure renderProcedure, const Map<uint32_t, sp<Uploader>>& uploaders = {}) const;

    class BUILDER_IMPL final : public Builder<Shader> {
    public:
        BUILDER_IMPL(BeanFactory& factory, const document& manifest, const ResourceLoaderContext& resourceLoaderContext, sp<Builder<Camera>> camera = nullptr,
                     Optional<std::vector<StageManifest>> stages = {}, Optional<SnippetManifest> snippets = {});

        sp<Shader> build(const Scope& args) override;

    private:
        sp<PipelineBuildingContext> makePipelineBuildingContext(const sp<Camera>& camera, const Scope& args) const;

    private:
        BeanFactory _factory;
        document _manifest;
        sp<RenderController> _render_controller;

        std::vector<StageManifest> _stages;
        SnippetManifest _snippets;
        SafeBuilder<Camera> _camera;
        PipelineDescriptor::Parameters::BUILDER _parameters;
    };

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<Shader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<Shader> build(const Scope& args) override;

    private:
        BUILDER_IMPL _impl;
    };

private:
    Map<uint32_t, Buffer> makeDivivedBuffers(const Map<uint32_t, sp<Uploader>>& uploaders) const;

private:
    sp<PipelineFactory> _pipeline_factory;
    sp<RenderController> _render_controller;
    sp<PipelineLayout> _pipeline_layout;
    sp<ShaderLayout> _pipeline_input;

    PipelineDescriptor::Parameters _descriptor_params;
};

}
