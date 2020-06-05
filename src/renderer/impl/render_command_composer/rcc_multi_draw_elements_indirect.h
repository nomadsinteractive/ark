#ifndef ARK_RENDERER_IMPL_RENDER_COMMAND_COMPOSER_RCC_MULTI_DRAW_ELEMENTS_INDIRECT_H_
#define ARK_RENDERER_IMPL_RENDER_COMMAND_COMPOSER_RCC_MULTI_DRAW_ELEMENTS_INDIRECT_H_

#include <unordered_map>

#include "core/collection/table.h"

#include "renderer/forwarding.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/model.h"
#include "renderer/inf/render_command_composer.h"
#include "renderer/inf/uploader.h"


namespace ark {

class ARK_API RCCMultiDrawElementsIndirect : public RenderCommandComposer {
public:
    RCCMultiDrawElementsIndirect(const sp<ModelBundle>& multiModels);

    virtual sp<ShaderBindings> makeShaderBindings(Shader& shader, RenderController& renderController, ModelLoader::RenderMode renderMode) override;

    virtual void postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot) override;
    virtual sp<RenderCommand> compose(const RenderRequest& renderRequest, RenderLayer::Snapshot& snapshot) override;

private:

    class VerticesUploader : public Uploader {
    public:
        VerticesUploader(const sp<ModelBundle>& multiModels, const sp<PipelineInput>& pipelineInput);

        void upload(Writable& uploader);

    private:
        sp<ModelBundle> _model_bundle;
        sp<PipelineInput> _pipeline_input;
    };

    class IndicesUploader : public Uploader {
    public:
        IndicesUploader(const sp<ModelBundle>& multiModels);

        void upload(Writable& uploader);

    private:
        sp<ModelBundle> _model_bundle;
    };

    struct IndirectCmd {
        DrawingContext::DrawElementsIndirectCommand _command;
        std::vector<size_t> _snapshot_offsets;
    };

private:
    sp<Uploader> makeIndirectBufferUploader();
    void writeModelMatices(const RenderRequest& renderRequest, DrawingBuffer& buf, const RenderLayer::Snapshot& snapshot, bool reload);

private:
    sp<ModelBundle> _model_bundle;

    Buffer _vertices;
    Buffer _indices;
    Buffer _draw_indirect;

    Table<int32_t, IndirectCmd> _indirect_cmds;
};

}

#endif
