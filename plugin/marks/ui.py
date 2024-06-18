import inspect
from typing import Callable, Any, Optional

import ark
from ark import Renderer, Vec2, Boolean, Vec3, Numeric, Vec4, String, Color, Integer, ApplicationFacade
from ark import dear_imgui
from marks import pydevd_start, pydevd_stop


class QuickBarItem:
    def __init__(self, text: str, on_click: Callable[[], None], close_main_window: bool = True):
        self._text = text
        self._on_click = on_click
        self._close_main_window = close_main_window

    @property
    def text(self) -> str:
        return self._text

    @property
    def on_click(self) -> Callable[[], None]:
        return self._on_click

    @property
    def close_main_window(self):
        return self._close_main_window


class InputField:
    INTPUT_TYPE_MAPPING = {Numeric: 'input_float', Vec2: 'input_float2', Vec3: 'input_float3', Vec4: 'input_float4', Color: 'color_edit4',
                                    Integer: 'input_int', Boolean: 'checkbox', String: 'input_text'}

    def __init__(self, name: str, value: Any, input_type: Optional[str] = None, *args, **kwargs):
        self._name = name
        self._input_type = input_type or self.INTPUT_TYPE_MAPPING[type(value)]
        self._value = value
        self._args = args
        self._kwargs = kwargs

    def build(self, builder: dear_imgui.WidgetBuilder):
        input_func = getattr(builder, self._input_type)
        input_func(self._name, self._value, *self._args, **self._kwargs)


class ConsoleCommand:
    def __init__(self, name: str, delegate):
        self._name = name
        self._packages = tuple(name.split('.'))
        self._delegate = delegate

    @property
    def name(self):
        return self._name

    @property
    def delegate(self):
        return self._delegate

    def get_executor(self, name_and_methods: list[str]):
        executor = None
        if self._packages == tuple(name_and_methods[:len(self._packages)]):
            remains = name_and_methods[len(self._packages):]
            executor = self._delegate
            while remains and executor:
                executor = getattr(executor, remains.pop(0), None)
        return executor

    def get_public_properties(self) -> list[tuple[str, Any]]:
        if isinstance(self._delegate, dict):
            return list(self._delegate.items())
        elif isinstance(self._delegate, list):
            return [(i.name, i.delegate) for i in self._delegate]

        pms = []
        for i, j in inspect.getmembers(self._delegate, lambda x: type(x) is not type):
            if not i.startswith('_'):
                pms.append((i, j))
        return pms

    @staticmethod
    def _get_public_methods(prefix: str, delegate) -> list[tuple[str, Callable]]:
        return [(f'{prefix}{i}', j) for i, j in inspect.getmembers(delegate, lambda x: callable(x)) if not i.startswith('_')]


class PropertyBundle:
    def __init__(self, input_fields: list[InputField]):
        self._items = []
        self._input_fields = input_fields

    @property
    def input_fields(self) -> list[InputField]:
        return self._items

    def add_property(self, name: str, value: Any, input_type: Optional[str] = None, *args):
        input_field = InputField(name, value, input_type, *args)
        self._items.append(input_field)
        self._input_fields.append(input_field)
        return value

    def dispose(self):
        for i in self._items:
            self._input_fields.remove(i)
        self._items = []


class VisibilityController:
    def __init__(self, visibility: Boolean):
        self._visibility = visibility

    def show(self):
        self._visibility.set(True)

    def hide(self):
        self._visibility.set(False)


class BaseWindow:
    def __init__(self, imgui: Renderer, is_open: Optional[bool]):
        self._imgui = imgui
        self._is_open = None if is_open is None else Boolean(is_open)
        self._renderer = Renderer()
        self._imgui.add_renderer(self._renderer)

    @property
    def is_open(self) -> Optional[Boolean]:
        return self._is_open

    def show(self, title: str, *args, **kwargs):
        if self._is_open is not None:
            self._is_open.set(True)

        builder = dear_imgui.WidgetBuilder(self._imgui)
        builder.begin(title, self._is_open)
        self.on_show(builder, *args, **kwargs)
        builder.end()
        self._renderer.reset(builder.make_widget().to_renderer())

    def hide(self):
        self._is_open.set(False)

    def on_show(self, builder: dear_imgui.WidgetBuilder, **kwargs):
        pass


class MainWindow(BaseWindow):

    class ConsoleCommand:
        def __init__(self, toolbox: 'MainWindow'):
            self._toolbox = toolbox

        def imgui_show_demo(self):
            self._toolbox._imgui_demo_is_open.set(True)

        def imgui_show_about(self):
            self._toolbox._imgui_about_is_open.set(True)

    def __init__(self, mark_studio: 'MarkStudio', imgui: Renderer, is_open: Optional[bool] = None, quick_bar_items: Optional[list[QuickBarItem]] = None):
        super().__init__(imgui, is_open)
        self._mark_studio = mark_studio
        self._quick_bar_items = quick_bar_items or []
        self._imgui_demo_is_open = Boolean(False)
        self._imgui_about_is_open = Boolean(False)
        self._pydevd_started = Boolean(False)
        self._quick_bar_widget = dear_imgui.Widget()

    @property
    def quick_bar_items(self) -> list[QuickBarItem]:
        return self._quick_bar_items

    @quick_bar_items.setter
    def quick_bar_items(self, quick_bar_items: Optional[list[QuickBarItem]] = None):
        self._quick_bar_items = quick_bar_items or []
        self._build_quick_bar()

    def on_show(self, builder: dear_imgui.WidgetBuilder):
        builder.text('Debugger')
        builder.small_button('Start').add_callback(self.pydevd_start)
        builder.same_line()
        builder.small_button('Stop').add_callback(self.pydevd_stop)
        builder.separator()

        builder.text('Quick Bar')
        builder.add_widget(self._quick_bar_widget)
        self._build_quick_bar()

        builder.separator()
        builder.small_button('Close').add_callback(self._mark_studio.close)

        builder.add_widget(builder.make_demo_widget(self._imgui_demo_is_open))
        builder.add_widget(builder.make_about_widget(self._imgui_about_is_open))

    def _build_quick_bar(self) -> dear_imgui.Widget:
        quick_bar_builder = dear_imgui.WidgetBuilder(self._imgui)

        for i, j in enumerate(self._quick_bar_items):
            if i != 0:
                quick_bar_builder.same_line()
            quick_bar_builder.small_button(j.text).add_callback(self._make_quickbar_onclick(j))

        self._quick_bar_widget.reset(quick_bar_builder.make_widget())

    def pydevd_start(self):
        pydevd_start()
        self._pydevd_started.set(True)
        self._mark_studio.close()

    def pydevd_stop(self):
        pydevd_stop()
        self._pydevd_started.set(False)
        self._mark_studio.close()

    def _make_quickbar_onclick(self, quick_bar_item: QuickBarItem):

        if quick_bar_item.close_main_window:
            def onclick():
                self._mark_studio.close()
                quick_bar_item.on_click()
            return onclick

        return quick_bar_item.on_click


class ConsoleWindow(BaseWindow):
    def __init__(self, imgui: Renderer, is_open: Optional[bool]):
        super().__init__(imgui, is_open)

    def on_show(self, builder: dear_imgui.WidgetBuilder, console_cmds: list[ConsoleCommand]):
        text_cmd = String('')

        def execute_cmd():
            cmd_splitted = text_cmd.val.split(' ')
            if cmd_splitted:
                name_and_methods = cmd_splitted[0].split('.')
                for i in console_cmds:
                    if executor := i.get_executor(name_and_methods):
                        executor(*cmd_splitted[1:])
            text_cmd.set('')

        builder.input_text('Command', text_cmd, 64, 1 << 5).add_callback(execute_cmd)
        builder.separator()
        builder.add_widget(self._make_cmd_tab_widget(console_cmds))

    def _make_cmd_tab_widget(self, console_cmds: list[ConsoleCommand]) -> dear_imgui.Widget:
        tab_title_builder = dear_imgui.WidgetBuilder(self._imgui)
        tab_panel_wrapper = dear_imgui.Widget()
        console_cmds_count = len(console_cmds)
        for i, j in enumerate(console_cmds):
            tab_title_builder.button(j.name).add_callback(self._make_tab_title_button_callback(j, tab_panel_wrapper))
            if i != console_cmds_count - 1:
                tab_title_builder.same_line()

        tab_title_builder.separator()
        tab_title_widget = tab_title_builder.make_widget()
        return tab_title_widget.before(tab_panel_wrapper)

    def _make_tab_panel_widget(self, console_cmd: ConsoleCommand, tab_panel: dear_imgui.Widget, panel_row_width_max: int = 80):
        builder = dear_imgui.WidgetBuilder(self._imgui)
        sub_tab_panel = dear_imgui.Widget()
        public_properties = console_cmd.get_public_properties()
        public_methods_len = len(public_properties)
        builder.push_id(id(console_cmd.delegate))
        panel_row_width = 0
        for i, j in enumerate(public_properties):
            cmd_name, cmd = j
            builder.button(cmd_name).add_callback(self._make_tab_panel_button_callback(cmd, tab_panel, sub_tab_panel))
            panel_row_width += (len(cmd_name) + 4)
            if i != public_methods_len - 1:
                if panel_row_width > panel_row_width_max:
                    panel_row_width = 0
                else:
                    builder.same_line()
        builder.pop_id()
        builder.add_widget(sub_tab_panel)
        return builder.make_widget()

    def _make_tab_title_button_callback(self, console_cmd: ConsoleCommand, tab_panel: dear_imgui.Widget):

        def _callback():
            tab_panel.reset(self._make_tab_panel_widget(console_cmd, tab_panel))

        return _callback

    def _make_tab_panel_button_callback(self, cmd: Any, tab_panel: dear_imgui.Widget, sub_tab_panel: dear_imgui.Widget):

        def _callback():
            result = cmd() if callable(cmd) else cmd
            if result is not None:
                builder = dear_imgui.WidgetBuilder(self._imgui)
                builder.separator()
                builder.add_widget(self._make_tab_panel_widget(ConsoleCommand('', result), tab_panel))
                sub_tab_panel.reset(builder.make_widget())

        return _callback


class PropertiesWindow(BaseWindow):
    def __init__(self, imgui: Renderer, is_open: Optional[bool]):
        super().__init__(imgui, is_open)

    def on_show(self, builder: dear_imgui.WidgetBuilder, properties: Any):
        if isinstance(properties, list):
            for i in properties:
                i.build(builder)
        else:
            properties.build(builder)


class ResourceWindow(BaseWindow):
    def __init__(self, imgui: Renderer, is_open: Optional[bool]):
        super().__init__(imgui, is_open)
        self._textures = []

    @property
    def textures(self):
        return self._textures

    def on_show(self, builder: dear_imgui.WidgetBuilder, **kwargs):
        for i in self._textures:
            builder.image(i)


class MarkStudio:

    class CommandDelegate:
        def __init__(self, mark_studio: 'MarkStudio', properties_visibility: Boolean):
            self._mark_studio = mark_studio
            self._properties = VisibilityController(properties_visibility)

        @property
        def properties(self):
            return self._properties

        def close(self):
            self._mark_studio.close()

    def __init__(self, application_facade: ApplicationFacade, imgui: Renderer, resolution: Vec2, quick_bar_items: Optional[list[QuickBarItem]] = None):
        self._application_facade = application_facade
        self._imgui = imgui
        self._discarded = None
        self._renderer_quickbar = None
        self._renderer_properties = None
        self._layer_editor_visibility = Boolean(False)
        self._main_window = MainWindow(self, self._imgui, None, quick_bar_items)
        self._console_window = ConsoleWindow(self._imgui, True)
        self._properties_window = PropertiesWindow(self._imgui, True)
        self._resource_window = ResourceWindow(self._imgui, True)
        self._resolution = resolution

    @property
    def main_window(self) -> MainWindow:
        return self._main_window

    @property
    def properties_window(self) -> PropertiesWindow:
        return self._properties_window

    @property
    def resource_window(self) -> ResourceWindow:
        return self._resource_window

    def show(self, console_cmds: Optional[list[ConsoleCommand]] = None, properties: Any = None):
        self._main_window.show('My Ark Studio')

        if properties:
            self._properties_window.show('Properties', properties)
        else:
            self._properties_window.hide()
        if console_cmds:
            self._console_window.show('Console', console_cmds)
        else:
            self._console_window.hide()

        if self._discarded is not None:
            self._discarded.set(True)
        self._discarded = Boolean(False)

        self._application_facade.surface_controller.add_control_layer(self._imgui, self._discarded)
        self._application_facade.push_event_listener(self._imgui, self._discarded)

    def close(self):
        self._imgui = None
        self._discarded.set(True)
        self._renderer_quickbar = None
        self._renderer_properties = None
        return True

    def make_default_command_items(self):
        return [ConsoleCommand('marks', MarkStudio.CommandDelegate(self, self._properties_window.is_open)),
                ConsoleCommand('tool_box', MainWindow.ConsoleCommand(self._main_window))]
