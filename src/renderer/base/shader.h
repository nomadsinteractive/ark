#ifndef ARK_RENDERER_BASE_SHADER_H_
#define ARK_RENDERER_BASE_SHADER_H_

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/render_layer.h"

#include "renderer/forwarding.h"
#include "renderer/inf/resource.h"
#include "renderer/inf/pipeline_factory.h"

namespace ark {

class ARK_API Shader {
public:
    enum Stage {
        STAGE_NONE = -1,
        STAGE_VERTEX,
        STAGE_FRAGMENT,
        STAGE_COUNT
    };

public:
    struct Stub : public PipelineFactory {
        Stub(const sp<PipelineFactory>& pipelineFactory);

        virtual sp<Pipeline> buildPipeline(GraphicsContext& graphicsContext, const sp<ShaderBindings>& shaderBindings) override;

        sp<Pipeline> _pipeline;
        sp<PipelineFactory> _pipeline_factory;
    };

public:
    Shader(const sp<Stub>& stub, const sp<PipelineLayout>& pipelineLayout, const sp<Camera>& camera);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Shader);

    static sp<Builder<Shader>> fromDocument(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& defVertex = "shaders/default.vert", const String& defFragment = "shaders/texture.frag");
    static sp<Shader> fromStringTable(const String& vertex = "shaders/default.vert", const String& fragment = "shaders/texture.frag", const sp<Snippet>& snippet = nullptr, const sp<ResourceLoaderContext>& resourceLoaderContext = nullptr);

    std::vector<RenderLayer::UBOSnapshot> snapshot(MemoryPool& memoryPool) const;

    const sp<PipelineInput>& input() const;
    const sp<Camera>& camera() const;

    const sp<Pipeline>& pipeline() const;
    const sp<PipelineFactory>& pipelineFactory() const;
    const sp<PipelineLayout>& pipelineLayout() const;

    const sp<Pipeline> getPipeline(GraphicsContext& graphicsContext, const sp<ShaderBindings>& bindings) const;

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<Shader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Shader> build(const sp<Scope>& args) override;

    private:
        BeanFactory _factory;
        document _manifest;
        sp<ResourceLoaderContext> _resource_loader_context;

        sp<Builder<String>> _vertex;
        sp<Builder<String>> _fragment;
        sp<Builder<Snippet>> _snippet;
        SafePtr<Builder<Camera>> _camera;
    };

private:
    sp<Stub> _stub;
    sp<PipelineLayout> _pipeline_layout;
    sp<PipelineInput> _input;
    sp<Camera> _camera;
};

}

#endif
