#ifndef ARK_RENDERER_BASE_SHADER_H_
#define ARK_RENDERER_BASE_SHADER_H_

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/layer.h"

#include "renderer/forwarding.h"
#include "renderer/inf/resource.h"

namespace ark {

class ARK_API Shader {
public:
    Shader(const sp<PipelineLayout>& pipelineLayout, const sp<Camera>& camera);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Shader);

    static sp<Builder<Shader>> fromDocument(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& defVertex = "shaders/default.vert", const String& defFragment = "shaders/texture.frag");
    static sp<Shader> fromStringTable(const String& vertex = "shaders/default.vert", const String& fragment = "shaders/texture.frag", const sp<Snippet>& snippet = nullptr, const sp<ResourceLoaderContext>& resourceLoaderContext = nullptr);

    Layer::UBOSnapshot snapshot(MemoryPool& memoryPool) const;

    void active(GraphicsContext& graphicsContext, const DrawingContext& drawingContext);

    const sp<PipelineInput>& input() const;
    const sp<Camera>& camera() const;

    const sp<Pipeline>& pipeline() const;
    const sp<PipelineFactory>& pipelineFactory() const;

    const sp<Pipeline>& getPipeline(GraphicsContext& graphicsContext);

    const sp<Snippet>& snippet() const;

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
        sp<Builder<Snippet>> _snippet;
        SafePtr<Builder<Camera>> _camera;
    };

private:
    struct Stub : public Resource {
        Stub(const sp<PipelineLayout>& pipelineLayout);

        virtual uint32_t id() override;
        virtual void upload(GraphicsContext& graphicsContext) override;
        virtual RecycleFunc recycle() override;

        sp<Pipeline> _pipeline;
        sp<PipelineFactory> _pipeline_factory;
        sp<PipelineLayout> _pipeline_layout;
        sp<PipelineInput> _input;
        sp<Snippet> _snippet;
    };

private:
    sp<Stub> _stub;
    sp<Camera> _camera;
};

}

#endif
