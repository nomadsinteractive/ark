#ifndef ARK_RENDERER_IMPL_RENDER_COMMAND_COMPOSER_RCC_MULTI_DRAW_ELEMENTS_INDIRECT_H_
#define ARK_RENDERER_IMPL_RENDER_COMMAND_COMPOSER_RCC_MULTI_DRAW_ELEMENTS_INDIRECT_H_

#include <vector>
#include <unordered_map>

#include "core/collection/table.h"

#include "renderer/forwarding.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/model.h"
#include "renderer/inf/render_command_composer.h"


namespace ark {

class ARK_API RCCMultiDrawElementsIndirect : public RenderCommandComposer {
public:
    RCCMultiDrawElementsIndirect(sp<ModelBundle> multiModels);

    virtual sp<ShaderBindings> makeShaderBindings(Shader& shader, RenderController& renderController, ModelLoader::RenderMode renderMode) override;

    virtual void postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot) override;
    virtual sp<RenderCommand> compose(const RenderRequest& renderRequest, RenderLayer::Snapshot& snapshot) override;

private:
    struct IndirectCmds {
        DrawingContext::DrawElementsIndirectCommand _command;
        std::vector<size_t> _snapshot_indices;
    };

private:
    ByteArray::Borrowed makeIndirectBuffer(const RenderRequest& renderRequest) const;
    void writeModelMatices(const RenderRequest& renderRequest, DrawingBuffer& buf, const RenderLayer::Snapshot& snapshot, bool reload);
    V3 toScale(const V3& size, const Metrics& metrics) const;

private:
    sp<ModelBundle> _model_bundle;

    Buffer _indices;
    Buffer _draw_indirect;

    Table<uint64_t, IndirectCmds> _indirect_cmds;
};

}

#endif
