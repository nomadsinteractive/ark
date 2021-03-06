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
#include "renderer/inf/model_loader.h"
#include "renderer/inf/resource.h"
#include "renderer/inf/pipeline_factory.h"

namespace ark {

//[[script::bindings::auto]]
class ARK_API Shader {
public:
    Shader(sp<PipelineFactory> pipelineFactory, sp<RenderController> renderController, sp<PipelineLayout> layout, const sp<Camera>& camera, PipelineBindings::Parameters bindingParams);
    DEFAULT_COPY_AND_ASSIGN(Shader);

    static sp<Builder<Shader>> fromDocument(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& defVertex = "shaders/default.vert", const String& defFragment = "shaders/texture.frag", const sp<Camera>& defaultCamera = nullptr);
    static sp<Shader> fromStringTable(const String& vertex = "shaders/default.vert", const String& fragment = "shaders/texture.frag", const sp<Snippet>& snippet = nullptr, const sp<ResourceLoaderContext>& resourceLoaderContext = nullptr);

    std::vector<RenderLayer::UBOSnapshot> takeUBOSnapshot(const RenderRequest& renderRequest) const;
    std::vector<Buffer::Snapshot> takeSSBOSnapshot(const RenderRequest& renderRequest) const;

    const sp<PipelineFactory>& pipelineFactory() const;
    const sp<RenderController>& renderController() const;

    const sp<PipelineInput>& input() const;
    const sp<PipelineLayout>& layout() const;

    sp<ShaderBindings> makeBindings(ModelLoader::RenderMode mode, PipelineBindings::RenderProcedure renderProcedure) const;

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<Shader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Shader> build(const Scope& args) override;

    private:
        std::map<PipelineInput::ShaderStage, sp<Builder<String>>> loadStages(BeanFactory& factory, const document& manifest) const;

        sp<PipelineBuildingContext> makePipelineBuildingContext(const Scope& args) const;

    private:
        BeanFactory _factory;
        document _manifest;
        sp<ResourceLoaderContext> _resource_loader_context;

        std::map<PipelineInput::ShaderStage, sp<Builder<String>>> _stages;
        std::vector<sp<Builder<Snippet>>> _snippets;
        SafePtr<Builder<Camera>> _camera;
        PipelineBindings::Parameters::BUILDER _parameters;
    };

private:
    sp<PipelineFactory> _pipeline_factory;
    sp<RenderController> _render_controller;
    sp<PipelineLayout> _pipeline_layout;
    sp<PipelineInput> _input;
    sp<Camera> _camera;

    PipelineBindings::Parameters _binding_params;
};

}

#endif
