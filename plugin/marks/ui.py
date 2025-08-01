import inspect
from collections.abc import Sequence
from typing import Callable, Any, Optional

from ark import Renderer, Vec2, Boolean, Vec3, Numeric, Vec4, String, Color, Integer, ApplicationFacade, Texture, Bitmap, Math, TYPE_BOOLEAN
from ark import dear_imgui


class QuickBarItem:
    def __init__(self, text: str, on_click: Callable[[], None]):
        self._text = text
        self._on_click = on_click

    @property
    def text(self) -> str:
        return self._text

    @property
    def on_click(self) -> Callable[[], None]:
        return self._on_click


class InputField:
    INTPUT_TYPE_MAPPING = {Numeric: 'input_float', Vec2: 'input_float2', Vec3: 'input_float3', Vec4: 'input_float4', Color: 'color_edit4',
                                    Integer: 'input_int', Boolean: 'checkbox', String: 'input_text'}

    def __init__(self, name: str, value: Any, input_type: Optional[str] = None, *args, **kwargs):
        self._name = name
        self._input_type = input_type or self.INTPUT_TYPE_MAPPING[type(value)]
        self._value = value
        self._args = args
        self._kwargs = kwargs

    @property
    def name(self) -> str:
        return self._name

    def build(self, builder: dear_imgui.WidgetBuilder):
        input_func = getattr(builder, self._input_type)
        input_func(self._name, self._value, *self._args, **self._kwargs)


class ConsoleCommand:
    def __init__(self, name: str, delegate: Any):
        self._name = name
        self._packages = tuple(name.split('.'))
        self._delegate = delegate

    @property
    def name(self):
        return self._name

    @property
    def delegate(self) -> Any:
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


class Window:
    def __init__(self, imgui: Renderer | None = None, is_open: Optional[TYPE_BOOLEAN] = None):
        self._imgui = imgui or _mark_studio.imgui
        self._is_open = None if is_open is None else Boolean(is_open)
        self._renderer = Renderer()
        self._imgui.add_renderer(self._renderer)

    @property
    def title(self) -> str:
        return ''

    @property
    def is_open(self) -> Optional[Boolean]:
        return self._is_open

    def show(self, args: Any = None):
        if self._is_open is not None:
            self._is_open.set(True)

        builder = dear_imgui.WidgetBuilder(self._imgui)
        builder.begin(self.title, self._is_open)
        self.on_show(builder, args)
        builder.end()
        self._renderer.reset(builder.make_widget().to_renderer())

    def hide(self):
        self._is_open.set(False)

    def on_show(self, builder: dear_imgui.WidgetBuilder, args: Any):
        pass


class MainWindow(Window):
    def __init__(self, mark_studio: "MarkStudio", is_open: Optional[TYPE_BOOLEAN] = None, quick_bar_items: Optional[list[QuickBarItem]] = None):
        super().__init__(mark_studio.imgui, is_open)
        self._mark_studio = mark_studio
        self._quick_bar_items = quick_bar_items or []
        self._imgui_demo_is_open = Boolean(False)
        self._imgui_about_is_open = Boolean(False)
        self._pydevd_started = Boolean(False)
        self._quick_bar_widget = dear_imgui.Widget()

    @property
    def title(self) -> str:
        return 'My Ark Studio'

    def on_show(self, builder: dear_imgui.WidgetBuilder, args: Any):
        builder.text('Windows')
        builder.small_button('Noise Generator').add_callback(self._show_noise_generator)
        builder.separator()
        builder.small_button('Imgui Demo').add_callback(self._show_imgui_demo)
        builder.same_line()
        builder.small_button('Imgui About').add_callback(self._show_imgui_about)
        self._build_quick_bar()

        builder.text('Quick Bar')
        builder.add_widget(self._quick_bar_widget)
        self._build_quick_bar()

        builder.separator()
        builder.small_button('Close').add_callback(self._mark_studio.close)

        builder.add_widget(builder.make_demo_widget(self._imgui_demo_is_open))
        builder.add_widget(builder.make_about_widget(self._imgui_about_is_open))

    def _build_quick_bar(self):
        quick_bar_builder = dear_imgui.WidgetBuilder(self._imgui)

        for i, j in enumerate(self._quick_bar_items):
            if i != 0:
                quick_bar_builder.same_line()
            quick_bar_builder.small_button(j.text).add_callback(self._make_quickbar_onclick(j))

        self._quick_bar_widget.reset(quick_bar_builder.make_widget())

    @staticmethod
    def _show_noise_generator():
        NoiseGeneratorWindow().show()

    def _show_imgui_demo(self):
        self._imgui_demo_is_open.set(True)

    def _show_imgui_about(self):
        self._imgui_about_is_open.set(True)

    def _make_quickbar_onclick(self, quick_bar_item: QuickBarItem):

        def onclick():
            self._mark_studio.close()
            quick_bar_item.on_click()

        return onclick


class ConsoleWindow(Window):
    def __init__(self, imgui: Renderer | None, is_open: Optional[bool]):
        super().__init__(imgui, is_open)

    @property
    def title(self) -> str:
        return 'Console'

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
        return dear_imgui.Widget([tab_title_widget, tab_panel_wrapper])

    def _make_tab_panel_widget(self, console_cmd: ConsoleCommand, tab_panel: dear_imgui.Widget):
        builder = dear_imgui.WidgetBuilder(self._imgui)
        sub_tab_panel = dear_imgui.Widget()
        public_properties = console_cmd.get_public_properties()
        builder.push_id(id(console_cmd.delegate))
        for i, j in enumerate(public_properties):
            cmd_name, cmd = j
            builder.button(cmd_name).add_callback(self._make_tab_panel_button_callback(cmd, tab_panel, sub_tab_panel))
            cra = builder.get_cursor_screen_pos() + builder.get_content_region_avail()
            item_rect_max = builder.get_item_rect_max()
            builder.same_line().visible_if(item_rect_max.x < cra.x - 80)
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
            if result is None:
                close_main_window()
            elif isinstance(result, Window):
                result.show()
            elif isinstance(result, dear_imgui.Widget):
                self._create_sub_panel(sub_tab_panel, result)
            elif not isinstance(result, bool):
                self._create_sub_panel(sub_tab_panel, self._make_tab_panel_widget(ConsoleCommand('', result), tab_panel))

        return _callback

    def _create_sub_panel(self, sub_tab_panel: dear_imgui.Widget, widget: dear_imgui.Widget):
        builder = dear_imgui.WidgetBuilder(self._imgui)
        builder.separator()
        builder.add_widget(widget)
        sub_tab_panel.reset(builder.make_widget())


class PropertiesWindow(Window):
    def __init__(self, imgui: Renderer | None = None, is_open: Optional[TYPE_BOOLEAN] = None, input_fields: Sequence[InputField] = None):
        super().__init__(imgui, is_open)
        self._input_fields = input_fields or []

    @property
    def title(self) -> str:
        return 'Properties'

    def on_show(self, builder: dear_imgui.WidgetBuilder, args: Any):
        for i in self._input_fields:
            i.build(builder)

        if isinstance(args, Sequence):
            for i in args:
                i.build(builder)


class NoiseGeneratorWindow(Window):
    def __init__(self, imgui: Renderer | None = None):
        super().__init__(imgui, True)
        try:
            from ark import noise
        except ImportError:
            noise = None
        self._noise = noise
        self._type = Integer(int(self._noise.Generator.NOISE_TYPE_PERLIN) if self._noise else 0)
        self._type_options = ['NOISE_TYPE_CELLULAR', 'NOISE_TYPE_SIMPLEX', 'NOISE_TYPE_PERLIN']
        self._components = Integer(1)
        self._size = Vec2(256, 256)
        self._position = Vec2(0, 0)
        self._frequency = Numeric(0.04)
        self._enable_fractal = Boolean(False)
        self._fractal_octaves = Integer(4)
        self._fractal_gain = Numeric(0.2)
        self._fractal_lacunarity = Numeric(1.0)
        self._texture = self._do_generate()

    @property
    def title(self) -> str:
        return 'Noise Generator'

    def on_show(self, builder: dear_imgui.WidgetBuilder, args: Any):
        builder.combo('Type', self._type, self._type_options)
        builder.slider_int('Components', self._components, 1, 4)
        builder.input_float2('Position', self._position)
        builder.input_float2('Size', self._size)
        builder.input_float('Frequency', self._frequency)
        builder.checkbox('Enable Fractal', self._enable_fractal)
        builder.slider_int('Fractal Octaves', self._fractal_octaves, 2, 8)
        builder.input_float('Fractal Gain', self._fractal_gain)
        builder.input_float('Fractal Lacunarity', self._fractal_lacunarity)
        if self._noise:
            builder.button('Generate').add_callback(lambda: self._texture.reset(self._do_generate()))
            builder.image(self._texture)
        else:
            builder.text_wrapped('Cannot import noise library, please make sure you have declared "ark-noise" and "ark-noise-pybindings" plugin in the ApplicationManifest')

    def _do_generate(self) -> Optional[Texture]:
        if not self._noise:
            return None

        generator_type = getattr(self._noise.Generator, self._type_options[self._type.val])
        x, y = (int(i) for i in self._position)
        width, height = (int(i) for i in self._size)

        float_arrays = []
        component_size = self._components.val
        for i in range(component_size):
            generator = self._noise.Generator(generator_type, Math.rand(), self._frequency.val)
            if self._enable_fractal:
                generator.set_fractal_octaves(self._fractal_octaves.val)
                generator.set_fractal_gain(self._fractal_gain.val)
                generator.set_fractal_lacunarity(self._fractal_lacunarity.val)
            float_arrays.append(generator.noise_map2d((x, y, x + width, y + height)))
        buf_arrays = float_arrays[0] if len(float_arrays) == 1 else float_arrays[0].intertwine(float_arrays[1:])
        bitmap = Bitmap(width, height, width * 4 * component_size, component_size, buf_arrays.to_byte_array())
        return Texture(bitmap, Texture.FORMAT_FLOAT | [Texture.FORMAT_R, Texture.FORMAT_RG, Texture.FORMAT_RGB, Texture.FORMAT_RGBA][component_size - 1])


class MarkStudio:
    def __init__(self, application_facade: ApplicationFacade, imgui: Renderer, resolution: Vec2, quick_bar_items: Optional[list[QuickBarItem]] = None):
        self._application_facade = application_facade
        self._imgui = imgui
        self._discarded = Boolean(False)
        self._renderer_quickbar = None
        self._renderer_properties = None
        self._main_window = MainWindow(self, None, quick_bar_items)
        self._console_window = ConsoleWindow(self._imgui, True)
        self._properties_window = PropertiesWindow(self._imgui, True)
        self._resolution = resolution

    @property
    def imgui(self) -> Renderer:
        return self._imgui

    @property
    def discarded(self) -> Boolean:
        return self._discarded

    def make_widget_builder(self) -> dear_imgui.WidgetBuilder:
        return dear_imgui.WidgetBuilder(self._imgui)

    def show_properties(self, properties: Any = None, **kwargs):
        if kwargs:
            input_fields = [InputField(k, v) for k, v in kwargs.items()]
            properties = [properties] + input_fields if properties else input_fields
        self.show(properties=properties)

    def show(self, console_cmds: Optional[list[ConsoleCommand]] = None, properties: Any = None):
        self._main_window.show()

        if properties:
            self._properties_window.show(properties)
        else:
            self._properties_window.hide()
        if console_cmds:
            self._console_window.show(console_cmds)
        else:
            self._console_window.hide()

        self._discarded.set(True)
        self._discarded = Boolean(False)

        self._application_facade.surface_controller.add_renderer(self._imgui, self._discarded, priority=Renderer.PRIORITY_CONTROL)
        self._application_facade.push_event_listener(self._imgui, self._discarded)

    def close(self):
        self._imgui = None
        self._discarded.set(True)
        self._renderer_quickbar = None
        self._renderer_properties = None
        return True


_mark_studio: Optional[MarkStudio] = None


def get_mark_studio() -> Optional[MarkStudio]:
    return _mark_studio


def close_main_window():
    global _mark_studio
    if _mark_studio and not _mark_studio.discarded:
        _mark_studio.close()
    _mark_studio = None


def show_main_window(application_facade: ApplicationFacade, imgui: Renderer, resolution: Vec2, quick_bar_items: Optional[list[QuickBarItem]] = None, console_cmds: Optional[list[ConsoleCommand]] = None):
    global _mark_studio
    if not _mark_studio or _mark_studio.discarded:
        _mark_studio = MarkStudio(application_facade, imgui, resolution, quick_bar_items)
        _mark_studio.show(console_cmds)
