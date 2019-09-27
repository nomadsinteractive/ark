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
#include "renderer/base/pipeline_bindings.h"
#include "renderer/inf/render_model.h"
#include "renderer/inf/resource.h"
#include "renderer/inf/pipeline_factory.h"

namespace ark {

//[[script::bindings::auto]]
class ARK_API Shader {
public:
    enum Stage {
        STAGE_NONE = -1,
        STAGE_VERTEX,
        STAGE_FRAGMENT,
        STAGE_COUNT
    };

public:
    Shader(const sp<PipelineFactory> pipelineFactory, const sp<RenderController>& renderController, const sp<PipelineLayout>& layout, const sp<Camera>& camera, PipelineBindings::Flag pipelineBindingsFlag);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Shader);

    static sp<Builder<Shader>> fromDocument(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& defVertex = "shaders/default.vert", const String& defFragment = "shaders/texture.frag", const sp<Camera>& defaultCamera = nullptr);
    static sp<Shader> fromStringTable(const String& vertex = "shaders/default.vert", const String& fragment = "shaders/texture.frag", const sp<Snippet>& snippet = nullptr, const sp<ResourceLoaderContext>& resourceLoaderContext = nullptr);

    std::vector<RenderLayer::UBOSnapshot> snapshot(MemoryPool& memoryPool) const;

    const sp<PipelineFactory>& pipelineFactory() const;
    const sp<RenderController>& renderController() const;

    const sp<PipelineInput>& input() const;
    const sp<PipelineLayout>& layout() const;
    const sp<Camera>& camera() const;

    sp<ShaderBindings> makeBindings(RenderModel::Mode mode) const;
    sp<ShaderBindings> makeBindings(RenderModel::Mode mode, const Buffer& vertex, const Buffer& index) const;

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<Shader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Shader> build(const Scope& args) override;

    private:
        BeanFactory _factory;
        document _manifest;
        sp<ResourceLoaderContext> _resource_loader_context;

        sp<Builder<String>> _vertex;
        sp<Builder<String>> _fragment;
        sp<Builder<Snippet>> _snippet;
        SafePtr<Builder<Camera>> _camera;
        PipelineBindings::Flag _pipeline_bindings_flag;
    };

private:
    sp<PipelineFactory> _pipeline_factory;
    sp<RenderController> _render_controller;
    sp<PipelineLayout> _pipeline_layout;
    sp<PipelineInput> _input;
    sp<Camera> _camera;
    PipelineBindings::Flag _pipeline_bindings_flag;
};

}

#endif
