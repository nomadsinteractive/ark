#pragma once

#include <imgui.h>

#include "core/forwarding.h"

#include "dear-imgui/forwarding.h"
#include "dear-imgui/api.h"

namespace ark::plugin::dear_imgui {

class ARK_PLUGIN_DEAR_IMGUI_API Imgui {
public:
//  [[script::bindings::enumeration]]
    enum ImGuiDataType {
        ImGuiDataType_S8 = ImGuiDataType_::ImGuiDataType_S8,
        ImGuiDataType_U8 = ImGuiDataType_::ImGuiDataType_U8,
        ImGuiDataType_S16 = ImGuiDataType_::ImGuiDataType_S16,
        ImGuiDataType_U16 = ImGuiDataType_::ImGuiDataType_U16,
        ImGuiDataType_S32 = ImGuiDataType_::ImGuiDataType_S32,
        ImGuiDataType_U32 = ImGuiDataType_::ImGuiDataType_U32,
        ImGuiDataType_S64 = ImGuiDataType_::ImGuiDataType_S64,
        ImGuiDataType_U64 = ImGuiDataType_::ImGuiDataType_U64,
        ImGuiDataType_Float = ImGuiDataType_::ImGuiDataType_Float,
        ImGuiDataType_Double = ImGuiDataType_::ImGuiDataType_Double,
        ImGuiDataType_Bool = ImGuiDataType_::ImGuiDataType_Bool,
        ImGuiDataType_String = ImGuiDataType_::ImGuiDataType_String,
        ImGuiDataType_COUNT = ImGuiDataType_::ImGuiDataType_COUNT
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

};

}
