#pragma once

#include "core/concurrent/lf_stack.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/pipeline_bindings.h"

#include "app/inf/event_listener.h"

#include "dear-imgui/forwarding.h"

namespace ark::plugin::dear_imgui {

class RendererImgui final : public Renderer, public EventListener {
public:
    RendererImgui(sp<Shader> shader, sp<Texture> texture);
    ~RendererImgui() override;

    void render(RenderRequest& renderRequest, const V3& position, const sp<DrawDecorator>& drawDecorator) override;
    bool onEvent(const Event& event) override;

    void addWidget(sp<Widget> widget, sp<Boolean> discarded);

    const sp<RendererContext>& rendererContext() const;

    struct DrawCommand {
        DrawCommand(RenderController& renderController);

        Buffer _vertex_buffer;
        Buffer _index_buffer;
        sp<Traits> _attachments;
    };

    class DrawCommandRecycler {
    public:
        DrawCommandRecycler(const sp<LFStack<sp<DrawCommand>>>& recycler, const sp<DrawCommand>& drawCommand);
        ~DrawCommandRecycler();

        const sp<DrawCommand>& drawCommand() const;

    private:
        sp<LFStack<sp<DrawCommand>>> _recycler;
        sp<DrawCommand> _draw_command;
    };

private:
    void MyImGuiRenderFunction(const RenderRequest& renderRequest, ImDrawData* draw_data) const;

private:
    sp<Shader> _shader;
    sp<RenderEngine> _render_engine;
    sp<Texture> _texture;

    sp<RendererContext> _renderer_context;

    D_FList<sp<Widget>> _widgets;
    Vector<std::pair<sp<Widget>, sp<Boolean>>> _widget_increasement;

    sp<BooleanWrapper> _text_input_enabled;
};

}
