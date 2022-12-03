from typing import Optional, Union

from ark import Renderer, Boolean, Observer, Text


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

    def begin(self, title: str):
        pass

    def end(self):
        pass

    def text(self, text: str):
        pass

    def bullet_text(self, text: str):
        pass

    def button(self, title: str) -> Observer:
        pass

    def input_text(self, label: str, text: Text, max_length: int = 64, flags: int = 0):
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
