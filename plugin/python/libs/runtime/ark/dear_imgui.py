from typing import Optional, Union

from ark import Renderer, Boolean, Observer, String, Integer, Numeric, Texture, Vec2, Vec4, Vec3


class Widget:
    def __init__(self, delegate: Optional['Widget'] = None):
        pass

    def reset(self, wrapped: Optional['Widget'] = None):
        pass

    def before(self, after: Optional['Widget']) -> 'Widget':
        pass

    def make_visible(self, visibility: Boolean) -> 'Widget':
        pass

    def to_renderer(self) -> Renderer:
        pass


class WidgetBuilder:
    def __init__(self, renderer):
        pass

    def begin(self, title: str, is_open: Optional[Boolean] = None):
        pass

    def end(self):
        pass

    def text(self, text: str):
        pass

    def bullet_text(self, text: str):
        pass

    def button(self, title: str) -> Observer:
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

    def same_line(self):
        pass

    def small_button(self, label: str) -> Observer:
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

    def push_id(self, hashid: Union[str, int]):
        pass

    def pop_id(self):
        pass

    def add_widget(self, widget: Widget):
        pass

    def make_widget(self) -> Widget:
        pass
