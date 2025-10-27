from collections.abc import Sequence
from typing import Optional, Union

from ark import Renderer, Boolean, Observer, String, Integer, Numeric, Texture, Vec2, Vec4, Vec3, TYPE_BOOLEAN, TYPE_VEC2


class ImGuiViewport:

    @property
    def id(self) -> Integer:
        pass

    @property
    def pos(self) -> Vec2:
        pass

    @property
    def size(self) -> Vec2:
        pass

    @property
    def work_pos(self) -> Vec2:
        pass

    @property
    def work_size(self) -> Vec2:
        pass


class Imgui:
    ImGuiWindowFlags_None = 0
    ImGuiWindowFlags_NoTitleBar = 0
    ImGuiWindowFlags_NoResize = 0
    ImGuiWindowFlags_NoMove = 0
    ImGuiWindowFlags_NoScrollbar = 0
    ImGuiWindowFlags_NoScrollWithMouse = 0
    ImGuiWindowFlags_NoCollapse = 0
    ImGuiWindowFlags_AlwaysAutoResize = 0
    ImGuiWindowFlags_NoBackground = 0
    ImGuiWindowFlags_NoSavedSettings = 0
    ImGuiWindowFlags_NoMouseInputs = 0
    ImGuiWindowFlags_MenuBar = 0
    ImGuiWindowFlags_HorizontalScrollbar = 0
    ImGuiWindowFlags_NoFocusOnAppearing = 0
    ImGuiWindowFlags_NoBringToFrontOnFocus = 0
    ImGuiWindowFlags_AlwaysVerticalScrollbar = 0
    ImGuiWindowFlags_AlwaysHorizontalScrollbar = 0
    ImGuiWindowFlags_NoNavInputs = 0
    ImGuiWindowFlags_NoNavFocus = 0
    ImGuiWindowFlags_UnsavedDocument = 0
    ImGuiWindowFlags_NoNav = 0
    ImGuiWindowFlags_NoDecoration = 0
    ImGuiWindowFlags_NoInputs = 0

    ImGuiInputTextFlags_None = 0
    ImGuiInputTextFlags_CharsDecimal = 0
    ImGuiInputTextFlags_CharsHexadecimal = 0
    ImGuiInputTextFlags_CharsScientific = 0
    ImGuiInputTextFlags_CharsUppercase = 0
    ImGuiInputTextFlags_CharsNoBlank = 0

    ImGuiInputTextFlags_AllowTabInput = 0
    ImGuiInputTextFlags_EnterReturnsTrue = 0
    ImGuiInputTextFlags_EscapeClearsAll = 0
    ImGuiInputTextFlags_CtrlEnterForNewLine = 0

    ImGuiInputTextFlags_ReadOnly = 0
    ImGuiInputTextFlags_Password = 0
    ImGuiInputTextFlags_AlwaysOverwrite = 0
    ImGuiInputTextFlags_AutoSelectAll = 0
    ImGuiInputTextFlags_ParseEmptyRefVal = 0
    ImGuiInputTextFlags_DisplayEmptyRefVal = 0
    ImGuiInputTextFlags_NoHorizontalScroll = 0
    ImGuiInputTextFlags_NoUndoRedo = 0

    ImGuiInputTextFlags_ElideLeft = 0

    ImGuiInputTextFlags_CallbackCompletion = 0
    ImGuiInputTextFlags_CallbackHistory = 0
    ImGuiInputTextFlags_CallbackAlways = 0
    ImGuiInputTextFlags_CallbackCharFilter = 0
    ImGuiInputTextFlags_CallbackResize = 0
    ImGuiInputTextFlags_CallbackEdit = 0

    ImGuiSliderFlags_None = 0
    ImGuiSliderFlags_Logarithmic = 0
    ImGuiSliderFlags_NoRoundToFormat = 0
    ImGuiSliderFlags_NoInput = 0
    ImGuiSliderFlags_WrapAround = 0
    ImGuiSliderFlags_ClampOnInput = 0
    ImGuiSliderFlags_ClampZeroRange = 0
    ImGuiSliderFlags_NoSpeedTweaks = 0
    ImGuiSliderFlags_AlwaysClamp = 0

    ImGuiColorEditFlags_None = 0
    ImGuiColorEditFlags_NoAlpha = 0
    ImGuiColorEditFlags_NoPicker = 0
    ImGuiColorEditFlags_NoOptions = 0
    ImGuiColorEditFlags_NoSmallPreview = 0
    ImGuiColorEditFlags_NoInputs = 0
    ImGuiColorEditFlags_NoTooltip = 0
    ImGuiColorEditFlags_NoLabel = 0
    ImGuiColorEditFlags_NoSidePreview = 0
    ImGuiColorEditFlags_NoDragDrop = 0
    ImGuiColorEditFlags_NoBorder = 0

    ImGuiColorEditFlags_AlphaOpaque = 0
    ImGuiColorEditFlags_AlphaNoBg = 0
    ImGuiColorEditFlags_AlphaPreviewHalf = 0

    ImGuiColorEditFlags_AlphaBar = 0
    ImGuiColorEditFlags_HDR = 0
    ImGuiColorEditFlags_DisplayRGB = 0
    ImGuiColorEditFlags_DisplayHSV = 0
    ImGuiColorEditFlags_DisplayHex = 0
    ImGuiColorEditFlags_Uint8 = 0
    ImGuiColorEditFlags_Float = 0
    ImGuiColorEditFlags_PickerHueBar = 0
    ImGuiColorEditFlags_PickerHueWheel = 0
    ImGuiColorEditFlags_InputRGB = 0
    ImGuiColorEditFlags_InputHSV = 0

    ImGuiTableFlags_None = 0
    ImGuiTableFlags_Resizable = 0
    ImGuiTableFlags_Reorderable = 0
    ImGuiTableFlags_Hideable = 0
    ImGuiTableFlags_Sortable = 0
    ImGuiTableFlags_NoSavedSettings = 0
    ImGuiTableFlags_ContextMenuInBody = 0

    ImGuiTableFlags_RowBg = 0
    ImGuiTableFlags_BordersInnerH = 0
    ImGuiTableFlags_BordersOuterH = 0
    ImGuiTableFlags_BordersInnerV = 0
    ImGuiTableFlags_BordersOuterV = 0
    ImGuiTableFlags_BordersH = 0
    ImGuiTableFlags_BordersV = 0
    ImGuiTableFlags_BordersInner = 0
    ImGuiTableFlags_BordersOuter = 0
    ImGuiTableFlags_Borders = 0
    ImGuiTableFlags_NoBordersInBody = 0
    ImGuiTableFlags_NoBordersInBodyUntilResize = 0

    ImGuiTableFlags_SizingFixedFit = 0
    ImGuiTableFlags_SizingFixedSame = 0
    ImGuiTableFlags_SizingStretchProp = 0
    ImGuiTableFlags_SizingStretchSame = 0

    ImGuiTableFlags_NoHostExtendX = 0
    ImGuiTableFlags_NoHostExtendY = 0
    ImGuiTableFlags_NoKeepColumnsVisible = 0
    ImGuiTableFlags_PreciseWidths = 0

    ImGuiTableFlags_NoClip = 0

    ImGuiTableFlags_PadOuterX = 0
    ImGuiTableFlags_NoPadOuterX = 0
    ImGuiTableFlags_NoPadInnerX = 0

    ImGuiTableFlags_ScrollX = 0
    ImGuiTableFlags_ScrollY = 0

    ImGuiTableFlags_SortMulti = 0
    ImGuiTableFlags_SortTristate = 0

    ImGuiTableFlags_HighlightHoveredColumn = 0

    ImGuiTabBarFlags_None = 0
    ImGuiTabBarFlags_Reorderable = 0
    ImGuiTabBarFlags_AutoSelectNewTabs = 0
    ImGuiTabBarFlags_TabListPopupButton = 0
    ImGuiTabBarFlags_NoCloseWithMiddleMouseButton = 0
    ImGuiTabBarFlags_NoTabListScrollingButtons = 0
    ImGuiTabBarFlags_NoTooltip = 0
    ImGuiTabBarFlags_DrawSelectedOverline = 0
    ImGuiTabBarFlags_FittingPolicyResizeDown = 0
    ImGuiTabBarFlags_FittingPolicyScroll = 0

    ImGuiTabItemFlags_None = 0
    ImGuiTabItemFlags_UnsavedDocument = 0
    ImGuiTabItemFlags_SetSelected = 0
    ImGuiTabItemFlags_NoCloseWithMiddleMouseButton = 0
    ImGuiTabItemFlags_NoPushId = 0
    ImGuiTabItemFlags_NoTooltip = 0
    ImGuiTabItemFlags_NoReorder = 0
    ImGuiTabItemFlags_Leading = 0
    ImGuiTabItemFlags_Trailing = 0
    ImGuiTabItemFlags_NoAssumedClosure = 0

    @staticmethod
    def get_main_viewport() -> ImGuiViewport:
        pass

    def add_widget(self, widget: "Widget", discarded: Optional[TYPE_BOOLEAN] = None):
        pass

    def show(self, discarded: Optional[TYPE_BOOLEAN]):
        pass


class Widget:
    def __init__(self, delegate: Optional["Widget", Sequence["Widget"]] = None):
        pass

    def reset(self, wrapped: Optional["Widget"] = None):
        pass

    def visible_if(self, visibility: Boolean):
        pass

    def to_renderer(self) -> Renderer:
        pass


class WidgetBuilder:
    def __init__(self, renderer):
        pass

    def set_next_window_pos(self, pos: TYPE_VEC2):
        pass

    def set_next_window_size(self, pos: TYPE_VEC2):
        pass

    def begin(self, title: str, is_open: Optional[Boolean] = None, flags: int = 0):
        pass

    def end(self):
        pass

    def get_cursor_screen_pos(self) -> Vec2:
        pass

    def get_content_region_avail(self) -> Vec2:
        pass

    def get_item_rect_min(self) -> Vec2:
        pass

    def get_item_rect_max(self) -> Vec2:
        pass

    def text(self, text: str):
        pass

    def text_wrapped(self, text: str):
        pass

    def bullet_text(self, text: str):
        pass

    def button(self, title: str) -> Observer:
        pass

    def color_edit3(self, label: str, value: Vec3, flags: int = 0):
        pass

    def color_edit4(self, label: str, value: Vec4, flags: int = 0):
        pass

    def color_picker3(self, label: str, value: Vec3, flags: int = 0):
        pass

    def color_picker4(self, label: str, value: Vec4, flags: int = 0):
        pass

    def slider_int(self, label: str, value: Integer, v_min: int, v_max: int, prompt_format: str = '%d'):
        pass

    def slider_float(self, label: str, value: Numeric, v_min: float, v_max: float, prompt_format: str = '%.3f'):
        pass

    def slider_float2(self, label: str, value: Vec2, v_min: float, v_max: float, prompt_format: str = '%.3f'):
        pass

    def slider_float3(self, label: str, value: Vec3, v_min: float, v_max: float, prompt_format: str = '%.3f'):
        pass

    def slider_float4(self, label: str, value: Vec4, v_min: float, v_max: float, prompt_format: str = '%.3f'):
        pass

    def slider_scalar(self, label: str, value: Vec2, v_min: TYPE_VEC2, v_max: TYPE_VEC2, format: Optional[str] = None):
        pass

    def input_float(self, label: str, v: Numeric, step: float = 0, step_fast: float = 0, format: str = '%.3f', extra_flags: int = 0):
        pass

    def input_float2(self, label: str, v: Vec2, step: float = 0, step_fast: float = 0, format: str = '%.3f', extra_flags: int = 0):
        pass

    def input_float3(self, label: str, v: Vec3, step: float = 0, step_fast: float = 0, format: str = '%.3f', extra_flags: int = 0):
        pass

    def input_float4(self, label: str, v: Vec4, step: float = 0, step_fast: float = 0, format: str = '%.3f', extra_flags: int = 0):
        pass

    def input_int(self, label: str, value: Integer, step: int = 1, step_fast: int = 100, flags: int = 0):
        pass

    def input_text(self, label: str, text: String, max_length: int = 64, flags: int = 0):
        pass

    def image(self, texture: Texture, size: Optional[Vec2] = None, uv0: tuple[float, float] = (0, 0), uv1: tuple[float, float] = (1, 1),
              color: Optional[Vec4] = None, border_color: Optional[Vec4] = None):
        pass

    def separator(self):
        pass

    def separator_text(self, label: str):
        pass

    def same_line(self, offset_from_start_x: float = 0, spacing: float = -1) -> Widget:
        pass

    def new_line(self) -> Widget:
        pass

    def small_button(self, label: str) -> Observer:
        pass

    def tree_node(self, label: str) -> Boolean:
        pass

    def tree_pop(self):
        pass

    def checkbox(self, label: str, option: Boolean):
        pass

    def radio_button(self, label: str, option: Integer, group: int):
        pass

    def combo(self, label: str, option: Integer, items: list[str]):
        pass

    def list_box(self, label: str, option: Integer, items: list[str]):
        pass

    def begin_tab_bar(self, str_id: str, flags: int = 0) -> Boolean:
        pass

    def end_tab_bar(self):
        pass

    def begin_tab_item(self, label: str, p_open: Optional[Boolean] = None, flags: int = 0) -> Boolean:
        pass

    def end_tab_item(self):
        pass

    def begin_main_menu_bar(self) -> Boolean:
        pass

    def end_main_menu_bar(self):
        pass

    def begin_menu(self, label: str, enabled: TYPE_BOOLEAN = True) -> Boolean:
        pass

    def end_menu(self):
        pass

    def menu_item(self, label: str, shortcut: str = '', p_selected: Optional[TYPE_BOOLEAN] = None, enabled: TYPE_BOOLEAN = True) -> Observer:
        pass

    def begin_table(self, str_id: str, columns: int, flags: int = 0) -> Boolean:
        pass

    def end_table(self):
        pass

    def table_setup_column(self, label: str):
        pass

    def table_headers_row(self):
        pass

    def table_next_row(self):
        pass

    def table_set_column_index(self, column_n: int):
        pass

    def push_id(self, hashid: Union[str, int]):
        pass

    def pop_id(self):
        pass

    def add_widget(self, widget: Widget):
        pass

    def make_widget(self) -> Widget:
        pass

    def make_demo_widget(self, is_open: TYPE_BOOLEAN = True) -> Widget:
        pass

    def make_about_widget(self, is_open: TYPE_BOOLEAN = True) -> Widget:
        pass
