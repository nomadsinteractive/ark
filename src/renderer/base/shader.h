#ifndef ARK_RENDERER_BASE_SHADER_H_
#define ARK_RENDERER_BASE_SHADER_H_

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/inf/render_resource.h"

namespace ark {

class ARK_API Shader {
public:
    Shader(const sp<PipelineLayout>& source, const sp<Camera>& camera);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Shader);

    static sp<Builder<Shader>> fromDocument(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& defVertex = "shaders/default.vert", const String& defFragment = "shaders/texture.frag");
    static sp<Shader> fromStringTable(const String& vertex = "shaders/default.vert", const String& fragment = "shaders/texture.frag", const sp<GLSnippet>& snippet = nullptr, const sp<ResourceLoaderContext>& resourceLoaderContext = nullptr);

    void use(GraphicsContext& graphicsContext);

    void bindUniforms(GraphicsContext& graphicsContext) const;

    const sp<PipelineInput>& input() const;

    const sp<Camera>& camera() const;

    const sp<Pipeline>& pipeline() const;
    const sp<PipelineFactory>& pipelineFactory() const;

    const sp<Pipeline>& getPipeline(GraphicsContext& graphicsContext);

    const sp<GLSnippet>& snippet() const;

//[[deprecated]]
    uint32_t stride() const;

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
        sp<Builder<GLSnippet>> _snippet;
        SafePtr<Builder<Camera>> _camera;
    };

private:
    struct Stub : public RenderResource {
        Stub(const sp<PipelineLayout>& pipelineLayout);

        virtual uint32_t id() override;
        virtual void prepare(GraphicsContext& graphicsContext) override;
        virtual void recycle(GraphicsContext& graphicsContext) override;

        sp<Pipeline> _pipeline;
        sp<PipelineFactory> _pipeline_factory;
        sp<PipelineLayout> _pipeline_layout;
        sp<PipelineInput> _input;
        sp<GLSnippet> _snippet;
    };

private:
    sp<Stub> _stub;
    sp<Camera> _camera;
};

}

#endif
