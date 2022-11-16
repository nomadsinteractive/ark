from ark import Renderer, Boolean, Observer


class Widget:

    def make_visible(self, visibility: Boolean) -> 'Widget':
        pass


class RendererBuilder:
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

    def separator(self):
        pass

    def same_line(self):
        pass

    def add_widget(self, widget: Widget):
        pass

    def make_widget(self) -> Widget:
        pass

    def make_renderer(self) -> Renderer:
        pass
