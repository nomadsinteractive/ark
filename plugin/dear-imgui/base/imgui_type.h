#pragma once

#include <imgui.h>

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"

#include "graphics/base/v2.h"

#include "dear-imgui/forwarding.h"
#include "dear-imgui/api.h"
#include "dear-imgui/renderer/renderer_imgui.h"

namespace ark::plugin::dear_imgui {

class ARK_PLUGIN_DEAR_IMGUI_API Imgui {
public:
//  [[script::bindings::enumeration]]
    enum ImGuiWindowFlags
    {
        ImGuiWindowFlags_None                   = ImGuiWindowFlags_::ImGuiWindowFlags_None,
        ImGuiWindowFlags_NoTitleBar             = ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar,
        ImGuiWindowFlags_NoResize               = ImGuiWindowFlags_::ImGuiWindowFlags_NoResize,
        ImGuiWindowFlags_NoMove                 = ImGuiWindowFlags_::ImGuiWindowFlags_NoMove,
        ImGuiWindowFlags_NoScrollbar            = ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollbar,
        ImGuiWindowFlags_NoScrollWithMouse      = ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollWithMouse,
        ImGuiWindowFlags_NoCollapse             = ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse,
        ImGuiWindowFlags_AlwaysAutoResize       = ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize,
        ImGuiWindowFlags_NoBackground           = ImGuiWindowFlags_::ImGuiWindowFlags_NoBackground,
        ImGuiWindowFlags_NoSavedSettings        = ImGuiWindowFlags_::ImGuiWindowFlags_NoSavedSettings,
        ImGuiWindowFlags_NoMouseInputs          = ImGuiWindowFlags_::ImGuiWindowFlags_NoMouseInputs,
        ImGuiWindowFlags_MenuBar                = ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar,
        ImGuiWindowFlags_HorizontalScrollbar    = ImGuiWindowFlags_::ImGuiWindowFlags_HorizontalScrollbar,
        ImGuiWindowFlags_NoFocusOnAppearing     = ImGuiWindowFlags_::ImGuiWindowFlags_NoFocusOnAppearing,
        ImGuiWindowFlags_NoBringToFrontOnFocus  = ImGuiWindowFlags_::ImGuiWindowFlags_NoBringToFrontOnFocus,
        ImGuiWindowFlags_AlwaysVerticalScrollbar= ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysVerticalScrollbar,
        ImGuiWindowFlags_AlwaysHorizontalScrollbar=ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysHorizontalScrollbar,
        ImGuiWindowFlags_NoNavInputs            = ImGuiWindowFlags_::ImGuiWindowFlags_NoNavInputs,
        ImGuiWindowFlags_NoNavFocus             = ImGuiWindowFlags_::ImGuiWindowFlags_NoNavFocus,
        ImGuiWindowFlags_UnsavedDocument        = ImGuiWindowFlags_::ImGuiWindowFlags_UnsavedDocument,
        ImGuiWindowFlags_NoNav                  = ImGuiWindowFlags_::ImGuiWindowFlags_NoNav,
        ImGuiWindowFlags_NoDecoration           = ImGuiWindowFlags_::ImGuiWindowFlags_NoDecoration,
        ImGuiWindowFlags_NoInputs               = ImGuiWindowFlags_::ImGuiWindowFlags_NoInputs
    };

//  [[script::bindings::enumeration]]
enum ImGuiInputTextFlags
{
    ImGuiInputTextFlags_None                = ImGuiInputTextFlags_::ImGuiInputTextFlags_None,
    ImGuiInputTextFlags_CharsDecimal        = ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsDecimal,
    ImGuiInputTextFlags_CharsHexadecimal    = ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsHexadecimal,
    ImGuiInputTextFlags_CharsScientific     = ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsScientific,
    ImGuiInputTextFlags_CharsUppercase      = ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsUppercase,
    ImGuiInputTextFlags_CharsNoBlank        = ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsNoBlank,

    ImGuiInputTextFlags_AllowTabInput       = ImGuiInputTextFlags_::ImGuiInputTextFlags_AllowTabInput,
    ImGuiInputTextFlags_EnterReturnsTrue    = ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue,
    ImGuiInputTextFlags_EscapeClearsAll     = ImGuiInputTextFlags_::ImGuiInputTextFlags_EscapeClearsAll,
    ImGuiInputTextFlags_CtrlEnterForNewLine = ImGuiInputTextFlags_::ImGuiInputTextFlags_CtrlEnterForNewLine,

    ImGuiInputTextFlags_ReadOnly            = ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly,
    ImGuiInputTextFlags_Password            = ImGuiInputTextFlags_::ImGuiInputTextFlags_Password,
    ImGuiInputTextFlags_AlwaysOverwrite     = ImGuiInputTextFlags_::ImGuiInputTextFlags_AlwaysOverwrite,
    ImGuiInputTextFlags_AutoSelectAll       = ImGuiInputTextFlags_::ImGuiInputTextFlags_AutoSelectAll,
    ImGuiInputTextFlags_ParseEmptyRefVal    = ImGuiInputTextFlags_::ImGuiInputTextFlags_ParseEmptyRefVal,
    ImGuiInputTextFlags_DisplayEmptyRefVal  = ImGuiInputTextFlags_::ImGuiInputTextFlags_DisplayEmptyRefVal,
    ImGuiInputTextFlags_NoHorizontalScroll  = ImGuiInputTextFlags_::ImGuiInputTextFlags_NoHorizontalScroll,
    ImGuiInputTextFlags_NoUndoRedo          = ImGuiInputTextFlags_::ImGuiInputTextFlags_NoUndoRedo,

    ImGuiInputTextFlags_ElideLeft			= ImGuiInputTextFlags_::ImGuiInputTextFlags_ElideLeft,

    ImGuiInputTextFlags_CallbackCompletion  = ImGuiInputTextFlags_::ImGuiInputTextFlags_CallbackCompletion,
    ImGuiInputTextFlags_CallbackHistory     = ImGuiInputTextFlags_::ImGuiInputTextFlags_CallbackHistory,
    ImGuiInputTextFlags_CallbackAlways      = ImGuiInputTextFlags_::ImGuiInputTextFlags_CallbackAlways,
    ImGuiInputTextFlags_CallbackCharFilter  = ImGuiInputTextFlags_::ImGuiInputTextFlags_CallbackCharFilter,
    ImGuiInputTextFlags_CallbackResize      = ImGuiInputTextFlags_::ImGuiInputTextFlags_CallbackResize,
    ImGuiInputTextFlags_CallbackEdit        = ImGuiInputTextFlags_::ImGuiInputTextFlags_CallbackEdit
};

//  [[script::bindings::enumeration]]
    enum ImGuiSliderFlags {
        ImGuiSliderFlags_None               = ImGuiSliderFlags_::ImGuiSliderFlags_None,
        ImGuiSliderFlags_Logarithmic        = ImGuiSliderFlags_::ImGuiSliderFlags_Logarithmic,
        ImGuiSliderFlags_NoRoundToFormat    = ImGuiSliderFlags_::ImGuiSliderFlags_NoRoundToFormat,
        ImGuiSliderFlags_NoInput            = ImGuiSliderFlags_::ImGuiSliderFlags_NoInput,
        ImGuiSliderFlags_WrapAround         = ImGuiSliderFlags_::ImGuiSliderFlags_WrapAround,
        ImGuiSliderFlags_ClampOnInput       = ImGuiSliderFlags_::ImGuiSliderFlags_ClampOnInput,
        ImGuiSliderFlags_ClampZeroRange     = ImGuiSliderFlags_::ImGuiSliderFlags_ClampZeroRange,
        ImGuiSliderFlags_NoSpeedTweaks      = ImGuiSliderFlags_::ImGuiSliderFlags_NoSpeedTweaks,
        ImGuiSliderFlags_AlwaysClamp        = ImGuiSliderFlags_::ImGuiSliderFlags_AlwaysClamp
    };

//  [[script::bindings::enumeration]]
enum ImGuiColorEditFlags {
    ImGuiColorEditFlags_None            = ImGuiColorEditFlags_::ImGuiColorEditFlags_None,
    ImGuiColorEditFlags_NoAlpha         = ImGuiColorEditFlags_::ImGuiColorEditFlags_NoAlpha,
    ImGuiColorEditFlags_NoPicker        = ImGuiColorEditFlags_::ImGuiColorEditFlags_NoPicker,
    ImGuiColorEditFlags_NoOptions       = ImGuiColorEditFlags_::ImGuiColorEditFlags_NoOptions,
    ImGuiColorEditFlags_NoSmallPreview  = ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSmallPreview,
    ImGuiColorEditFlags_NoInputs        = ImGuiColorEditFlags_::ImGuiColorEditFlags_NoInputs,
    ImGuiColorEditFlags_NoTooltip       = ImGuiColorEditFlags_::ImGuiColorEditFlags_NoTooltip,
    ImGuiColorEditFlags_NoLabel         = ImGuiColorEditFlags_::ImGuiColorEditFlags_NoLabel,
    ImGuiColorEditFlags_NoSidePreview   = ImGuiColorEditFlags_::ImGuiColorEditFlags_NoSidePreview,
    ImGuiColorEditFlags_NoDragDrop      = ImGuiColorEditFlags_::ImGuiColorEditFlags_NoDragDrop,
    ImGuiColorEditFlags_NoBorder        = ImGuiColorEditFlags_::ImGuiColorEditFlags_NoBorder,

    ImGuiColorEditFlags_AlphaOpaque     = ImGuiColorEditFlags_::ImGuiColorEditFlags_AlphaOpaque,
    ImGuiColorEditFlags_AlphaNoBg       = ImGuiColorEditFlags_::ImGuiColorEditFlags_AlphaNoBg,
    ImGuiColorEditFlags_AlphaPreviewHalf= ImGuiColorEditFlags_::ImGuiColorEditFlags_AlphaPreviewHalf,

    ImGuiColorEditFlags_AlphaBar        = ImGuiColorEditFlags_::ImGuiColorEditFlags_AlphaBar,
    ImGuiColorEditFlags_HDR             = ImGuiColorEditFlags_::ImGuiColorEditFlags_HDR,
    ImGuiColorEditFlags_DisplayRGB      = ImGuiColorEditFlags_::ImGuiColorEditFlags_DisplayRGB,
    ImGuiColorEditFlags_DisplayHSV      = ImGuiColorEditFlags_::ImGuiColorEditFlags_DisplayHSV,
    ImGuiColorEditFlags_DisplayHex      = ImGuiColorEditFlags_::ImGuiColorEditFlags_DisplayHex,
    ImGuiColorEditFlags_Uint8           = ImGuiColorEditFlags_::ImGuiColorEditFlags_Uint8,
    ImGuiColorEditFlags_Float           = ImGuiColorEditFlags_::ImGuiColorEditFlags_Float,
    ImGuiColorEditFlags_PickerHueBar    = ImGuiColorEditFlags_::ImGuiColorEditFlags_PickerHueBar,
    ImGuiColorEditFlags_PickerHueWheel  = ImGuiColorEditFlags_::ImGuiColorEditFlags_PickerHueWheel,
    ImGuiColorEditFlags_InputRGB        = ImGuiColorEditFlags_::ImGuiColorEditFlags_InputRGB,
    ImGuiColorEditFlags_InputHSV        = ImGuiColorEditFlags_::ImGuiColorEditFlags_InputHSV
};

//  [[script::bindings::enumeration]]
enum ImGuiTableFlags
{
    ImGuiTableFlags_None                       = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_Resizable                  = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_Reorderable                = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_Hideable                   = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_Sortable                   = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_NoSavedSettings            = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_ContextMenuInBody          = ImGuiTableFlags_::ImGuiTableFlags_None,

    ImGuiTableFlags_RowBg                      = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_BordersInnerH              = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_BordersOuterH              = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_BordersInnerV              = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_BordersOuterV              = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_BordersH                   = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_BordersV                   = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_BordersInner               = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_BordersOuter               = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_Borders                    = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_NoBordersInBody            = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_NoBordersInBodyUntilResize = ImGuiTableFlags_::ImGuiTableFlags_None,

    ImGuiTableFlags_SizingFixedFit             = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_SizingFixedSame            = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_SizingStretchProp          = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_SizingStretchSame          = ImGuiTableFlags_::ImGuiTableFlags_None,

    ImGuiTableFlags_NoHostExtendX              = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_NoHostExtendY              = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_NoKeepColumnsVisible       = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_PreciseWidths              = ImGuiTableFlags_::ImGuiTableFlags_None,

    ImGuiTableFlags_NoClip                     = ImGuiTableFlags_::ImGuiTableFlags_None,

    ImGuiTableFlags_PadOuterX                  = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_NoPadOuterX                = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_NoPadInnerX                = ImGuiTableFlags_::ImGuiTableFlags_None,

    ImGuiTableFlags_ScrollX                    = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_ScrollY                    = ImGuiTableFlags_::ImGuiTableFlags_None,

    ImGuiTableFlags_SortMulti                  = ImGuiTableFlags_::ImGuiTableFlags_None,
    ImGuiTableFlags_SortTristate               = ImGuiTableFlags_::ImGuiTableFlags_None,

    ImGuiTableFlags_HighlightHoveredColumn     = ImGuiTableFlags_::ImGuiTableFlags_None,
};

//  [[script::bindings::enumeration]]
enum ImGuiTabBarFlags
{
    ImGuiTabBarFlags_None                           = ImGuiTabBarFlags_::ImGuiTabBarFlags_None,
    ImGuiTabBarFlags_Reorderable                    = ImGuiTabBarFlags_::ImGuiTabBarFlags_Reorderable,
    ImGuiTabBarFlags_AutoSelectNewTabs              = ImGuiTabBarFlags_::ImGuiTabBarFlags_AutoSelectNewTabs,
    ImGuiTabBarFlags_TabListPopupButton             = ImGuiTabBarFlags_::ImGuiTabBarFlags_TabListPopupButton,
    ImGuiTabBarFlags_NoCloseWithMiddleMouseButton   = ImGuiTabBarFlags_::ImGuiTabBarFlags_NoCloseWithMiddleMouseButton,
    ImGuiTabBarFlags_NoTabListScrollingButtons      = ImGuiTabBarFlags_::ImGuiTabBarFlags_NoTabListScrollingButtons,
    ImGuiTabBarFlags_NoTooltip                      = ImGuiTabBarFlags_::ImGuiTabBarFlags_NoTooltip,
    ImGuiTabBarFlags_DrawSelectedOverline           = ImGuiTabBarFlags_::ImGuiTabBarFlags_DrawSelectedOverline,
    ImGuiTabBarFlags_FittingPolicyResizeDown        = ImGuiTabBarFlags_::ImGuiTabBarFlags_FittingPolicyResizeDown,
    ImGuiTabBarFlags_FittingPolicyScroll            = ImGuiTabBarFlags_::ImGuiTabBarFlags_FittingPolicyScroll
};

//  [[script::bindings::enumeration]]
enum ImGuiTabItemFlags
{
    ImGuiTabItemFlags_None                          = ImGuiTabItemFlags_::ImGuiTabItemFlags_None,
    ImGuiTabItemFlags_UnsavedDocument               = ImGuiTabItemFlags_::ImGuiTabItemFlags_UnsavedDocument,
    ImGuiTabItemFlags_SetSelected                   = ImGuiTabItemFlags_::ImGuiTabItemFlags_SetSelected,
    ImGuiTabItemFlags_NoCloseWithMiddleMouseButton  = ImGuiTabItemFlags_::ImGuiTabItemFlags_NoCloseWithMiddleMouseButton,
    ImGuiTabItemFlags_NoPushId                      = ImGuiTabItemFlags_::ImGuiTabItemFlags_NoPushId,
    ImGuiTabItemFlags_NoTooltip                     = ImGuiTabItemFlags_::ImGuiTabItemFlags_NoTooltip,
    ImGuiTabItemFlags_NoReorder                     = ImGuiTabItemFlags_::ImGuiTabItemFlags_NoReorder,
    ImGuiTabItemFlags_Leading                       = ImGuiTabItemFlags_::ImGuiTabItemFlags_Leading,
    ImGuiTabItemFlags_Trailing                      = ImGuiTabItemFlags_::ImGuiTabItemFlags_Trailing,
    ImGuiTabItemFlags_NoAssumedClosure              = ImGuiTabItemFlags_::ImGuiTabItemFlags_NoAssumedClosure
};

public:
    Imgui(sp<RendererImgui> renderer);

//  [[script::bindings::auto]]
    static sp<ImGuiViewportType> getMainViewport();

//  [[script::bindings::auto]]
    void addWidget(sp<Widget> widget, sp<Boolean> discarded) const;
//  [[script::bindings::auto]]
    void addRenderer(sp<Renderer> renderer, sp<Boolean> discarded) const;
//  [[script::bindings::auto]]
    void show(sp<Boolean> discarded) const;

//  [[plugin::builder("imgui")]]
    class BUILDER final : public Builder<Imgui> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Imgui> build(const Scope& args) override;

    private:
        document _manifest;
        sp<Camera> _camera;
        sp<Builder<Shader>> _shader;
    };

private:
    sp<RendererImgui> _renderer;

    friend class WidgetBuilder;
};

}
