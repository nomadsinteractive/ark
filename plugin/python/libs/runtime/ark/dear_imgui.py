from typing import Optional, Union

from ark import Renderer, Boolean, Observer, String, Integer, Numeric, Texture, Vec2, Vec4


class Widget:
    def __init__(self, delegate: Optional['Widget'] = None):
        pass

    @property
    def wrapped(self) -> Optional['Widget']:
        return None

    @wrapped.setter
    def wrapped(self, wrapped: Optional['Widget']):
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

    def slider_int(self, label: str, value: Integer, min_value: int, max_value: int, prompt_format: str = '%d'):
        pass

    def slider_float(self, label: str, value: Numeric, min_value: float, max_value: float, prompt_format: str = '%.3f'):
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

    def push_id(self, hashid: Union[str, int]):
        pass

    def pop_id(self):
        pass

    def add_widget(self, widget: Widget):
        pass

    def make_widget(self) -> Widget:
        pass
