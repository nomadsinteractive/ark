#include "dear-imgui/base/imgui_type.h"

#include "widget_type.h"
#include "app/base/application_context.h"
#include "app/base/application_facade.h"
#include "core/ark.h"
#include "core/inf/asset.h"
#include "core/inf/readable.h"
#include "core/types/global.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/surface_controller.h"

#include "renderer/base/shader.h"
#include "renderer/base/render_controller.h"

#include "dear-imgui/base/imgui_viewport_type.h"
#include "dear-imgui/base/imgui_context.h"
#include "dear-imgui/renderer/renderer_imgui.h"

namespace ark::plugin::dear_imgui {

Imgui::Imgui(sp<RendererImgui> renderer)
    : _renderer(std::move(renderer))
{
}

sp<ImGuiViewportType> Imgui::getMainViewport()
{
    return sp<ImGuiViewportType>::make(ImGui::GetMainViewport());
}

void Imgui::addWidget(sp<Widget> widget, sp<Boolean> discarded) const
{
    _renderer->addRenderer(WidgetType::toRenderer(std::move(widget)), {std::move(discarded)});
}

void Imgui::addRenderer(sp<Renderer> renderer, sp<Boolean> discarded) const
{
    _renderer->addRenderer(std::move(renderer), {std::move(discarded)});
}

void Imgui::show(sp<Boolean> discarded) const
{
    const sp<ApplicationFacade>& facade = Ark::instance().applicationContext()->applicationFacade();
    facade->surfaceController()->addRenderer(_renderer, discarded);
    facade->pushEventListener(_renderer, std::move(discarded));
}

Imgui::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _manifest(manifest), _camera(sp<Camera>::make(Ark::instance().createCamera(enums::COORDINATE_SYSTEM_LHS, Ark::instance().renderController()->renderEngine()->isBackendLHS()))),
      _shader(Shader::fromDocument(factory, manifest, "shaders/imgui.vert", "shaders/imgui.frag"))
{
}

sp<Imgui> Imgui::BUILDER::build(const Scope& args)
{
    const Global<ImguiContext> context;
    const ImGuiIO& io = ImGui::GetIO();

    for(const document& i : _manifest->children("font"))
    {
        ImFontConfig fontConfig;
        const String& src = Documents::getAttribute(i, constants::SRC);
        fontConfig.SizePixels = Documents::getAttribute<float>(i, constants::SIZE, 18.0f);
        if(src.empty())
            io.Fonts->AddFontDefault(&fontConfig);
        else
        {
            const sp<Asset> asset = Ark::instance().getAsset(src);
            DCHECK(asset, "Font \"%s\" not found", src.c_str());
            const sp<Readable> readable = asset->open();
            const int32_t datasize = readable->remaining();
            void* data = malloc(static_cast<size_t>(datasize));
            readable->read(data, static_cast<uint32_t>(datasize));
            strncpy(fontConfig.Name, src.c_str(), sizeof(fontConfig.Name));
            io.Fonts->AddFontFromMemoryTTF(data, datasize, fontConfig.SizePixels, &fontConfig);
        }
    }

    unsigned char* pixels = nullptr;
    int32_t width, height, bytesPerPixel;

    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytesPerPixel);
    RenderController& renderController = Ark::instance().renderController();
    sp<Bitmap> bitmap = sp<Bitmap>::make(width, height, bytesPerPixel * width, 4, sp<ByteArray>::make<ByteArray::Borrowed>(pixels, width * height * bytesPerPixel));
    sp<Texture> texture = renderController.createTexture2d(std::move(bitmap));
    sp<Shader> shader = _shader->build(args);
    const Viewport& viewport = renderController.renderEngine()->viewport();
    _camera->ortho(0, viewport.width(), viewport.height(), 0, viewport.clipNear(), viewport.clipFar());
    shader->setCamera(_camera);
    return sp<Imgui>::make(sp<RendererImgui>::make(std::move(shader), std::move(texture)));
}

}
