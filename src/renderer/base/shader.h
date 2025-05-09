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

//[[script::bindings::class]]
class ARK_API Shader {
public:
    struct StageManifest {
        StageManifest(enums::ShaderStageBit type, builder<String> source);
        StageManifest(BeanFactory& factory, const document& manifest);

        enums::ShaderStageBit _type;
        builder<String> _source;
        document _manifest;
    };

    typedef Vector<sp<Builder<Snippet>>> SnippetManifest;

public:
    Shader(sp<PipelineDescriptor> pipelineDescriptor);

    static sp<Builder<Shader>> fromDocument(BeanFactory& factory, const document& manifest, const String& defVertex = "shaders/default.vert", const String& defFragment = "shaders/texture.frag", const sp<Camera>& defaultCamera = nullptr);

    sp<RenderBufferSnapshot> takeBufferSnapshot(const RenderRequest& renderRequest, bool isComputeStage) const;

    const Camera& camera() const;
    void setCamera(const Camera& camera);

    const sp<PipelineDescriptor>& pipelineDesciptor() const;
    const sp<PipelineLayout>& layout() const;

    sp<PipelineBindings> makeBindings(Buffer vertexBuffer, enums::DrawMode drawMode, enums::DrawProcedure drawProcedure, Vector<std::pair<uint32_t, Buffer>> instanceBuffers = {}) const;

    class BUILDER_IMPL final : public Builder<Shader> {
    public:
        BUILDER_IMPL(BeanFactory& factory, const document& manifest, sp<Builder<Camera>> camera = nullptr, Optional<Vector<StageManifest>> stages = {}, Optional<SnippetManifest> snippets = {});

        sp<Shader> build(const Scope& args) override;

    private:
        sp<PipelineBuildingContext> makePipelineBuildingContext(const Scope& args) const;

    private:
        BeanFactory _factory;
        document _manifest;

        Vector<StageManifest> _stages;
        SnippetManifest _snippets;
        SafeBuilder<Camera> _camera;
        PipelineDescriptor::Configuration::BUILDER _configuration;
    };

//  [[plugin::builder]]
    class BUILDER final : public Builder<Shader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Shader> build(const Scope& args) override;

    private:
        BUILDER_IMPL _impl;
    };

private:
    sp<PipelineDescriptor> _pipeline_desciptor;
};

}
