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
        StageManifest(enums::ShaderStageBit type, sp<IBuilder<String>> source);
        StageManifest(BeanFactory& factory, const document& manifest);

        enums::ShaderStageBit _type;
        sp<IBuilder<String>> _source;
        document _manifest;
    };

    typedef Vector<sp<Builder<Snippet>>> SnippetManifest;

public:
    Shader(sp<PipelineDescriptor> pipelineDescriptor);

    sp<RenderBufferSnapshot> takeBufferSnapshot(const RenderRequest& renderRequest, bool isComputeStage) const;

    const Camera& camera() const;
    void setCamera(const Camera& camera);

    const sp<PipelineDescriptor>& pipelineDesciptor() const;
    const sp<PipelineLayout>& layout() const;

    sp<PipelineBindings> makeBindings(Buffer vertexBuffer, enums::DrawMode drawMode, enums::DrawProcedure drawProcedure, Vector<std::pair<uint32_t, Buffer>> instanceBuffers = {}) const;

//  [[plugin::builder]]
    class BUILDER final : public Builder<Shader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, Vector<StageManifest> defaultStages = {});

        sp<Shader> build(const Scope& args) override;
        sp<Shader> build(const Scope& args, const Vector<StageManifest>& defaultStages, sp<Camera> defaultCamera);

    private:
        PipelineBuildingContext makePipelineBuildingContext(const Scope& args, const Vector<StageManifest>& defaultStages) const;

    private:
        BeanFactory _factory;
        document _manifest;

        Vector<StageManifest> _stages;
        Vector<StageManifest> _default_stages;
        SnippetManifest _snippets;
        SafeBuilder<Camera> _camera;
        PipelineDescriptor::Configuration::BUILDER _configuration;

        friend class Shader;
    };

    static sp<Builder<Shader>> makeBuilder(BeanFactory& factory, const document& manifest, const String& defVertex = "shaders/default.vert", const String& defFragment = "shaders/texture.frag");

private:
    sp<PipelineDescriptor> _pipeline_desciptor;
};

}
