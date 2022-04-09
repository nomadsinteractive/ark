"""

This module only servers as a mimic of the Ark runtime library.
Functionalities are limited or unimplemented.

Use it for:
1. Autocomplete
2. Documentation
3. Unit test(maybe)

"""

from typing import Callable, List, Type, TypeVar, Union, Optional, Dict, Tuple

_BUILDABLE_TYPES = TypeVar('_BUILDABLE_TYPES', 'Arena', 'AudioPlayer', 'Boolean', 'Characters', 'Collider', 'Integer', 'Numeric', 'NarrowPhrase', 'Layer',
                           'Vec2', 'Vec3', 'Vec4', 'Renderer', 'RenderLayer', 'RenderObject', 'Rotation', 'Size', 'StringBundle', 'Tilemap', 'TilemapImporter',
                           'Tileset', 'TilesetImporter', 'Transform', 'Varyings')


TYPE_INT_OR_FLOAT = Union[int, float]
TYPE_NUMERIC = Union[TYPE_INT_OR_FLOAT, 'Numeric']
TYPE_RECT = Tuple[TYPE_INT_OR_FLOAT, TYPE_INT_OR_FLOAT, TYPE_INT_OR_FLOAT, TYPE_INT_OR_FLOAT]
TYPE_VEC2 = Union[Tuple[TYPE_NUMERIC, TYPE_NUMERIC], 'Vec2']
TYPE_VEC3 = Union[Tuple[TYPE_NUMERIC, TYPE_NUMERIC, TYPE_NUMERIC], 'Vec3']


def logd(*args):
    print(*args)


def logw(*args):
    print(*args)


def loge(*args):
    print(*args)


def load_asset(filename) -> str:
    with open(filename, 'rt') as fp:
        return fp.read()


def open_asset(filename):
    return load_asset(filename)


def get_ref_manager():
    return None


def is_debug_build():
    return True


class _ArkObject:
    pass


class Object:
    pass


class Readable:
    pass


class _Var:
    def __init__(self, v):
        pass

    @property
    def val(self):
        return None

    @val.setter
    def val(self, v):
        pass

    @property
    def delegate(self):
        return None

    @delegate.setter
    def delegate(self, v):
        pass

    def set(self, v):
        pass

    def fix(self):
        pass

    def freeze(self):
        pass

    def wrap(self):
        pass

    def periodic(self, interval: Optional['Numeric'] = None, duration: Optional['Numeric'] = None):
        pass

    def __int__(self):
        return 0

    def __add__(self, other):
        return 0

    def __radd__(self, other):
        pass

    def __mul__(self, other):
        pass

    def __rmul__(self, other):
        pass

    def __sub__(self, other):
        pass

    def __rsub__(self, other):
        pass

    def __truediv__(self, other):
        pass

    def __gt__(self, other) -> 'Boolean':
        return Boolean(False)

    def __lt__(self, other) -> 'Boolean':
        return Boolean(False)

    def __eq__(self, other) -> 'Boolean':
        return Boolean(False)

    def __float__(self):
        return 0.0


class MessageLoop:

    def post(self, task: Callable[[], None], delay: float):
        pass

    def schedule(self, task: Callable[[], None], interval: float):
        pass


class ApplicationFacade:

    def __init__(self):
        self._arena = None

    @property
    def clock(self) -> 'Clock':
        return Clock()

    @property
    def cursor_position(self) -> 'Vec2':
        return Vec2(0, 0)

    @property
    def application_controller(self) -> 'ApplicationController':
        return ApplicationController()

    @property
    def surface_controller(self) -> 'SurfaceController':
        return SurfaceController()

    @property
    def camera(self) -> 'Camera':
        return Camera()

    @property
    def manifest(self) -> 'Manifest':
        return Manifest()

    @property
    def resource_loader(self) -> 'ResourceLoader':
        return ResourceLoader()

    @property
    def arena(self) -> 'Arena':
        return self._arena

    @arena.setter
    def arena(self, arena: 'Arena'):
        self._arena = arena

    @property
    def argv(self) -> List[str]:
        return []

    def add_string_bundle(self, name: str, string_bundle: 'StringBundle'):
        pass

    def create_resource_loader(self, name: str, **kwargs) -> 'ResourceLoader':
        pass

    def make_message_loop(self, clock: 'Clock') -> MessageLoop:
        pass

    def add_pre_render_task(self, task: Callable, expired: 'Boolean' = None):
        pass

    def add_control_layer(self, control: Union['Layer', 'Renderer']):
        pass

    def add_event_listener(self, event_listener: Callable[['Event'], bool], priority: int = 0):
        pass

    def set_default_event_listener(self, event_listener: Callable[['Event'], bool]):
        pass

    def get_string(self, resid: str) -> str:
        pass

    def get_string_array(self, resid: str) -> List[str]:
        pass

    def exit(self):
        pass

    def post(self, task: Callable, delay: float):
        pass

    def schedule(self, task: Callable, interval: float):
        pass


class ApplicationController:

    SYSTEM_CURSOR_ARROW = 0
    SYSTEM_CURSOR_IBEAM = 1
    SYSTEM_CURSOR_WAIT = 2
    SYSTEM_CURSOR_CROSSHAIR = 3
    SYSTEM_CURSOR_WAITARROW = 4
    SYSTEM_CURSOR_SIZENWSE = 5
    SYSTEM_CURSOR_SIZENESW = 6
    SYSTEM_CURSOR_SIZEWE = 7
    SYSTEM_CURSOR_SIZENS = 8
    SYSTEM_CURSOR_SIZEALL = 9
    SYSTEM_CURSOR_NO = 10
    SYSTEM_CURSOR_HAND = 11

    def create_cursor(self, bitmap: 'Bitmap', x: int, y: int):
        pass

    def create_system_cursor(self, name: int):
        pass

    def show_cursor(self, cursor: Optional = None):
        pass

    def hide_cursor(self):
        pass

    def set_mouse_capture(self, enabled: bool):
        pass

    def exit(self):
        pass


class AtlasImporter:
    pass


class Atlas:

    def has(self, c: int) -> bool:
        pass

    def get_original_size(self, c:  int) -> tuple:
        pass

    def add_importer(self, importer: AtlasImporter, readable: Optional[Readable] = None):
        pass


class Buffer:

    @property
    def size(self) -> int:
        return 0

    @property
    def id(self) -> int:
        return 0


class Future:
    def __init__(self):
        pass

    def cancel(self):
        pass

    def done(self):
        pass

    @property
    def is_cancelled(self) -> bool:
        return False

    @property
    def is_done(self) -> bool:
        return False


class BroadPhrase:
    pass


class AudioPlayer:
    PLAY_OPTION_DEFAULT = -1
    PLAY_OPTION_LOOP_OFF = 0
    PLAY_OPTION_LOOP_ON = 1

    def play(self, source: Union[str, Readable], options: int = PLAY_OPTION_DEFAULT):
        pass


class Bitmap:

    @property
    def width(self) -> int:
        return 0

    @property
    def height(self) -> int:
        return 0

    @property
    def depth(self) -> int:
        return 0

    @property
    def row_bytes(self) -> int:
        return 0

    @property
    def bytes(self):
        return None


class Manifest:

    @property
    def name(self) -> str:
        return ''

    @property
    def renderer_resolution(self) -> 'Size':
        return Size(0, 0)


class Observer:
    def __init__(self, callback, oneshot: bool = True):
        pass

    def update(self):
        pass


class Shader:
    pass


class Renderer:
    def __init__(self, delegate: Optional['Renderer'] = None):
        self._delegate = delegate

    def add_renderer(self, renderer: 'Renderer'):
        pass

    def dispose(self):
        pass

    @property
    def size(self) -> 'Size':
        return Size(0, 0)

    @property
    def delegate(self) -> 'Renderer':
        return self

    @delegate.setter
    def delegate(self, v: 'Renderer'):
        pass

    def translate(self, position: Union[tuple, 'Vec2']) -> 'Renderer':
        return self

    def make_disposable(self, disposed: Optional[bool, 'Disposed'] = None) -> 'Renderer':
        pass

    def make_visible(self, visibility: Union[bool, 'Boolean', 'Visibility']) -> 'Renderer':
        pass

    def make_auto_release(self, ref_count: int = 1) -> 'Renderer':
        pass


class ResourceLoader:

    def load(self, cls: Type[_BUILDABLE_TYPES], type_id: str, **kwargs) -> _BUILDABLE_TYPES:
        return cls()

    @property
    def refs(self):
        return None

    @property
    def layers(self):
        return None

    @property
    def render_layers(self):
        return None

    @property
    def packages(self):
        return None

    def get_reference(self, name: str):
        return None


class Camera:
    def __init__(self):
        pass

    @property
    def position(self) -> 'Vec3':
        return None

    @property
    def matrix_view(self) -> 'Mat4':
        return None

    @property
    def matrix_projection(self) -> 'Mat4':
        return None

    @property
    def matrix_view_projection(self) -> 'Mat4':
        return None

    def ortho(self, left_top: TYPE_VEC2, right_bottom: TYPE_VEC2, clip: TYPE_VEC2):
        pass

    def frustum(self, left, right, top, bottom, near, far):
        pass

    def perspective(self, fov, aspect, near, far):
        pass

    def look_at(self, position, target, up):
        pass

    def to_world_position(self, screen_x: float, screen_y: float, z: float) -> tuple:
        pass

    def to_screen_position(self, position: TYPE_VEC3) -> 'Vec3':
        pass


class Boolean(_Var):
    def __init__(self, value):
        _Var.__init__(self, value)

    def ternary(self, positive, negative):
        pass

    def toggle(self):
        pass

    def negative(self) -> 'Boolean':
        pass

    def __or__(self, other) -> 'Boolean':
        pass

    def __and__(self, other) -> 'Boolean':
        pass


class Numeric(_Var):
    def __init__(self, val):
        _Var.__init__(self, val)

    def approach(self, expectation) -> 'ExpectationF':
        return ExpectationF(self)

    def at_least(self, least) -> 'ExpectationF':
        return ExpectationF(self)

    def at_most(self, most) -> 'ExpectationF':
        return ExpectationF(self)

    def boundary(self, boundary) -> 'ExpectationF':
        return ExpectationF(self)

    def fence(self, fence) -> 'ExpectationF':
        return ExpectationF(self)

    def integral(self, t: Optional['Numeric'] = None) -> 'Numeric':
        pass

    def integral_with_resistance(self, v0: float, cd: Union[float, 'Numeric'], t: Optional['Numeric'] = None) -> 'Numeric':
        pass

    def if_else(self, condition: Union[bool, Boolean], negative: Union[float, 'Numeric']) -> 'Numeric':
        pass

    def synchronize(self, disposed: Boolean) -> 'Numeric':
        pass

    @staticmethod
    def pursue(from_value: float, to_value, duration: float, t=None):
        return None

    @staticmethod
    def vibrate(s0: float, v0: float, s1: float, v1: float, duration: float, t=None):
        return None

    def __floordiv__(self, other):
        pass


class Integer(_Var):
    REPEAT_NONE = 0
    REPEAT_LAST = 1
    REPEAT_RESTART = 2
    REPEAT_REVERSE = 3
    REPEAT_REVERSE_RESTART = 4

    def __init__(self, value: Union[int, 'Integer', Numeric]):
        super().__init__(value)

    def __int__(self) -> int:
        return 0

    @staticmethod
    def repeat(array: List[int], repeat: int) -> 'ExpectationI':
        pass

    def if_else(self, condition: Boolean, negative: 'Integer') -> 'Integer':
        pass

    def animate(self, interval: Union[Numeric, float] = None, duration: Union[Numeric, float] = None) -> 'Integer':
        pass


class Disposed:
    def __init__(self, disposed=False):
        pass

    def dispose(self):
        pass

    def observe(self, observer: Observer) -> Boolean:
        return Boolean(False)

    def to_boolean(self) -> Boolean:
        return Boolean(False)

    def __bool__(self):
        return False


class Vec2:
    def __init__(self, x, y):
        self._x = x
        self._y = y

    @property
    def x(self) -> float:
        return self._x

    @x.setter
    def x(self, v):
        pass

    @property
    def y(self) -> float:
        return self._y

    @property
    def xy(self):
        return self._x, self._y

    @xy.setter
    def xy(self, val):
        pass

    @y.setter
    def y(self, v):
        pass

    @property
    def vx(self) -> Numeric:
        return self._x

    @vx.setter
    def vx(self, vx):
        pass

    @property
    def vy(self) -> Numeric:
        return self._y

    @vy.setter
    def vy(self, vy):
        pass

    @property
    def delegate(self):
        return self

    def set(self, val):
        pass

    def transform(self, transform):
        return self._x, self._y

    def integral(self, t: Optional[Numeric] = None) -> 'Vec2':
        pass

    def integral_with_resistance(self, v0: Tuple[float, float], cd: Union[float, 'Numeric'], t: Optional['Numeric'] = None) -> 'Vec2':
        pass

    def wrap(self) -> 'Vec2':
        return Vec2(self._x, self._y)

    def synchronize(self, disposed: Optional[Boolean] = None) -> 'Vec2':
        return Vec2(self._x, self._y)

    def attract(self, s0: Union[tuple, 'Vec2'], duration: float, t: Optional[Numeric] = None) -> 'Vec2':
        pass

    def fence(self, plane: Union['Vec3', tuple], observer: Union[Observer, Callable[[], None]]) -> 'Vec2':
        return Vec2(self._x, self._y)

    def fix(self):
        pass

    def freeze(self):
        pass

    def atan2(self) -> Numeric:
        pass

    def normalize(self):
        pass

    def distance_to(self, other) -> Numeric:
        pass

    def __add__(self, other):
        return None

    def __sub__(self, other):
        return None

    def __mul__(self, other):
        return None

    def __truediv__(self, other):
        return None

    def __str__(self):
        return '(%.1f, %.1f)' % (self._x, self._y)


class Vec3(Vec2):
    def __init__(self, x, y, z):
        Vec2.__init__(self, x, y)
        self._z = z

    @property
    def z(self):
        return self._z

    @z.setter
    def z(self, v):
        pass

    @property
    def vz(self):
        return self._z

    @vz.setter
    def vz(self, vz):
        pass

    @property
    def xyz(self):
        return None

    @xyz.setter
    def xyz(self, v):
        pass

    def cross(self, other) -> 'Vec3':
        pass


class Vec4(Vec3):
    def __init__(self, x, y, z, w):
        Vec3.__init__(self, x, y, z)
        self._w = w

    @property
    def w(self):
        return self._w

    @w.setter
    def w(self, v):
        pass

    @property
    def vw(self):
        return self._z

    @vw.setter
    def vw(self, v):
        pass


class Mat2:
    def __init__(self, t=None, b=None):
        pass

    def __mul__(self, other):
        pass

    def __matmul__(self, other):
        pass

    def rotate(self, rot: Union[float, Numeric, 'Rotation']) -> 'Mat2':
        pass

    def freeze(self) -> 'Mat2':
        pass

    def if_else(self, condition: Boolean, other: 'Mat2') -> 'Mat2':
        pass


class Mat3:
    def __init__(self, t=None, b=None, n=None):
        pass

    def __mul__(self, other):
        return None

    def __matmul__(self, other):
        pass


class Mat4:
    def __init__(self, t=None, b=None, n=None, w=None):
        pass

    def __mul__(self, other):
        pass

    def __matmul__(self, other):
        pass


class Input:
    def __init__(self, delegate: 'Input'):
        pass

    @property
    def size(self):
        return 0

    def set(self, delegate: 'Input'):
        pass


class AnimationInput(Input):

    def get_node_matrix(self, node_name: str) -> Mat4:
        pass


class Animation:

    @property
    def duration(self) -> float:
        return 0

    @property
    def node_names(self) -> List[str]:
        return []

    def make_transforms(self, t: Numeric, on_complete) -> AnimationInput:
        pass


class Model:

    @property
    def index_length(self) -> int:
        return 0

    @property
    def vertex_length(self) -> int:
        return 0

    @property
    def animations(self) -> Dict[str, Animation]:
        return {}

    def get_animation(self, name: str) -> Animation:
        pass


class ModelBundle:

    def load(self, t: int) -> Model:
        pass


class Level:

    def load(self, src: str):
        pass

    def get_render_object(self, name: str) -> 'RenderObject':
        pass

    def get_rigid_body(self, name: str) -> 'RigidBody':
        pass


class RenderObject:
    def __init__(self, t, pos=None, size=None, transform=None, varyings: Optional['Varyings'] = None, visible: Optional['Visibility'] = None, disposed: Optional[Disposed] = None):
        self._position = pos
        self._size = size
        self._transform = transform
        self._disposed = Boolean(False)

    @property
    def type(self):
        return 0

    @type.setter
    def type(self, val):
        pass

    @property
    def position(self) -> Vec3:
        return self._position

    @position.setter
    def position(self, v):
        pass

    @property
    def disposed(self) -> Boolean:
        return self._disposed

    @disposed.setter
    def disposed(self, disposed: Boolean):
        pass

    @property
    def tag(self):
        return None

    @tag.setter
    def tag(self, tag):
        pass

    @property
    def x(self) -> float:
        return 0

    @property
    def y(self) -> float:
        return 0

    @property
    def z(self) -> float:
        return 0

    @property
    def size(self) -> 'Size':
        return self._size

    @size.setter
    def size(self, v: 'Size'):
        self._size = v

    @property
    def transform(self) -> 'Transform':
        return self._transform

    @transform.setter
    def transform(self, transform):
        self._transform = transform

    @property
    def varyings(self) -> 'Varyings':
        return Varyings()

    @varyings.setter
    def varyings(self, v):
        pass

    @property
    def visible(self) -> Boolean:
        return Boolean(True)

    @visible.setter
    def visible(self, val: Union[Boolean, bool]):
        pass

    @property
    def xy(self) -> Tuple[float, float]:
        return 0, 0

    @property
    def xyz(self) -> Tuple[float, float, float]:
        return 0, 0, 0

    def absorb(self, o):
        pass

    def dispose(self):
        pass

    def show(self):
        pass

    def hide(self):
        pass


class _Expectation:
    def create_observer(self, callback: Callable, oneshot=True) -> Observer:
        pass

    def add_observer(self, callback: Callable, oneshot=True) -> Observer:
        pass

    def clear(self):
        pass


class ExpectationF(Numeric, _Expectation):
    def __init__(self, val):
        super().__init__(val)


class ExpectationI(Integer, _Expectation):
    def __init__(self, val):
        super().__init__(val)


class LayerContext:
    pass


class RenderLayer(Renderer):

    @property
    def context(self) -> LayerContext:
        return LayerContext()

    @property
    def layer(self) -> 'Layer':
        return Layer(self)

    def make_layer(self, layer_type: int) -> 'Layer':
        pass

    def make_context(self, layer_type) -> LayerContext:
        pass


class Layer(Renderer):
    TYPE_UNSPECIFIED = 0
    TYPE_DYNAMIC = 1
    TYPE_STATIC = 2

    @property
    def context(self) -> LayerContext:
        return LayerContext()

    def add_render_object(self, render_object: RenderObject, disposed: Boolean = None):
        pass

    def clear(self):
        pass


class Arena:

    def load(self, clz: Type[_BUILDABLE_TYPES], name, **kwargs) -> _BUILDABLE_TYPES:
        return clz()

    @property
    def layers(self):
        return None

    @property
    def render_layers(self):
        return None

    @property
    def packages(self):
        return None

    @property
    def refs(self):
        return None

    def get_reference(self, name: str):
        pass

    @property
    def resource_loader(self):
        return None

    def add_renderer(self, renderer: Renderer):
        pass

    def add_layer(self, layer: Layer):
        pass

    def add_render_layer(self, render_layer: RenderLayer):
        pass

    def load_renderer(self, name: str, **kwargs):
        pass

    def add_event_listener(self, event_listener: Union[Callable[['Event'], bool], 'EventListener'], priority: int = 0):
        pass


class Clock:

    @property
    def tick(self) -> int:
        return 0

    def duration(self) -> 'Numeric':
        pass

    def timeout(self, seconds: float) -> 'Boolean':
        pass

    def pause(self):
        pass

    def resume(self):
        pass


class Event:
    ACTION_KEY_NONE = 0
    ACTION_KEY_DOWN = 1
    ACTION_KEY_UP = 2
    ACTION_KEY_REPEAT = 3
    ACTION_DOWN = 4
    ACTION_UP = 5
    ACTION_MOVE = 6
    ACTION_WHEEL = 7
    ACTION_BACK_PRESSED = 8
    ACTION_CANCEL = 9

    CODE_NONE = 0
    CODE_MOUSE_BUTTON_LEFT = 1
    CODE_MOUSE_BUTTON_MIDDLE = 2
    CODE_MOUSE_BUTTON_RIGHT = 3
    CODE_KEYBOARD_A = 1
    CODE_KEYBOARD_B = 2
    CODE_KEYBOARD_C = 3
    CODE_KEYBOARD_D = 4
    CODE_KEYBOARD_E = 5
    CODE_KEYBOARD_F = 6
    CODE_KEYBOARD_G = 7
    CODE_KEYBOARD_H = 8
    CODE_KEYBOARD_I = 9
    CODE_KEYBOARD_J = 10
    CODE_KEYBOARD_K = 11
    CODE_KEYBOARD_L = 12
    CODE_KEYBOARD_M = 13
    CODE_KEYBOARD_N = 14
    CODE_KEYBOARD_O = 15
    CODE_KEYBOARD_P = 16
    CODE_KEYBOARD_Q = 17
    CODE_KEYBOARD_R = 18
    CODE_KEYBOARD_S = 19
    CODE_KEYBOARD_T = 20
    CODE_KEYBOARD_U = 21
    CODE_KEYBOARD_V = 22
    CODE_KEYBOARD_W = 23
    CODE_KEYBOARD_X = 24
    CODE_KEYBOARD_Y = 25
    CODE_KEYBOARD_Z = 26
    CODE_KEYBOARD_F1 = 27
    CODE_KEYBOARD_F2 = 28
    CODE_KEYBOARD_F3 = 29
    CODE_KEYBOARD_F4 = 30
    CODE_KEYBOARD_F5 = 31
    CODE_KEYBOARD_F6 = 32
    CODE_KEYBOARD_F7 = 33
    CODE_KEYBOARD_F8 = 34
    CODE_KEYBOARD_F9 = 35
    CODE_KEYBOARD_F10 = 36
    CODE_KEYBOARD_F11 = 37
    CODE_KEYBOARD_F12 = 38
    CODE_KEYBOARD_GRAVE = 39
    CODE_KEYBOARD_RIGHT = 40
    CODE_KEYBOARD_LEFT = 41
    CODE_KEYBOARD_DOWN = 42
    CODE_KEYBOARD_UP = 43
    CODE_KEYBOARD_ENTER = 44
    CODE_KEYBOARD_ESCAPE = 45
    CODE_KEYBOARD_BACKSPACE = 46
    CODE_KEYBOARD_TAB = 47
    CODE_KEYBOARD_SPACE = 48

    CODE_KEYBOARD_BACK = 49
    CODE_KEYBOARD_MENU = 50

    @property
    def action(self) -> int:
        return self.ACTION_KEY_DOWN

    @property
    def code(self) -> int:
        return self.CODE_NONE

    @property
    def x(self) -> int:
        return 0

    @property
    def y(self) -> int:
        return 0

    @property
    def xy(self) -> tuple:
        return 0, 0

    @property
    def timestamp(self) -> int:
        return 0


class Rotation:
    def __init__(self, theta: Union[float, Numeric], axis: Optional[Vec3, tuple] = None):
        self._theta = theta
        self._axis = axis

    @property
    def theta(self) -> Numeric:
        return self._theta

    @theta.setter
    def theta(self, theta: Union[float, Numeric]):
        pass

    @property
    def axis(self) -> Vec3:
        return self._axis

    def set_rotation(self, theta: Union[float, Numeric], axis: Optional[Vec3, tuple]):
        pass

    def set_euler(self, pitch: Union[float, Numeric], yaw: Union[float, Numeric], roll: Union[float, Numeric]):
        pass


class Transform:
    TYPE_LINEAR_2D = 0
    TYPE_LINEAR_3D = 1

    def __init__(self, t: int = TYPE_LINEAR_3D, rotation: Optional[Rotation] = None, scale: Optional[Vec3, Vec2] = None, pivot: Optional[Vec3, Vec2] = None):
        self._type = t
        self._rotation = rotation
        self._scale = scale
        self._pivot = pivot

    @property
    def rotation(self) -> Rotation:
        return self._rotation

    @rotation.setter
    def rotation(self, v: Rotation):
        self._rotation = v

    @property
    def scale(self) -> Vec3:
        return self._scale

    @scale.setter
    def scale(self, v):
        self._scale = v

    @property
    def pivot(self) -> Vec3:
        return self._pivot

    @pivot.setter
    def pivot(self, v):
        self._pivot = v

    def to_matrix(self) -> Mat4:
        pass


class Random:
    def __init__(self, seed: Optional[int] = None):
        self._seed = seed

    @property
    def seed(self):
        return self._seed

    @seed.setter
    def seed(self, seed):
        self._seed = seed

    def randf(self) -> float:
        return 0

    def randv(self) -> Numeric:
        return 0

    def uniform(self, a: Union[float, Numeric], b: Union[float, Numeric]) -> Numeric:
        return Numeric(0)

    def normal(self, a: Union[float, Numeric], b: Union[float, Numeric]) -> Numeric:
        return Numeric(0)


class Math:

    @staticmethod
    def abs(x):
        return x

    @staticmethod
    def sin(x):
        return 0

    @staticmethod
    def cos(x):
        return 0

    @staticmethod
    def max(a, b):
        return 0

    @staticmethod
    def min(a, b):
        return 0

    @staticmethod
    def sqrt(x):
        return 0

    @staticmethod
    def hypot(x, y):
        return 0

    @staticmethod
    def dot(x, y) -> Numeric:
        pass

    @staticmethod
    def distance(a, b):
        pass

    @staticmethod
    def randf() -> float:
        return 0

    @staticmethod
    def randv() -> Numeric:
        return Numeric(0)

    @staticmethod
    def atan(x):
        return 0

    @staticmethod
    def atan2(dy, dx):
        return 0

    @staticmethod
    def lerp(a, b, t):
        return 0

    @staticmethod
    def quadratic(a, b, c):
        return 0, 0

    @staticmethod
    def projectile(dx, dy, v, g, sid):
        return 0, 0


class Size:
    def __init__(self, w, h, d = None):
        pass

    @property
    def width(self):
        return 0

    @width.setter
    def width(self, width):
        pass

    @property
    def height(self):
        return 0

    @height.setter
    def height(self, height):
        pass


class Tile:
    def __init__(self, _id: int, _type: int = 0, render_object: Optional[RenderObject] = None):
        pass

    @property
    def id(self) -> int:
        return 0

    @property
    def type(self) -> int:
        return 0

    @type.setter
    def type(self, t):
        pass

    @property
    def render_object(self) -> Optional[RenderObject]:
        return None

    @render_object.setter
    def render_object(self, render_object: Optional[RenderObject]):
        pass


class TilemapImporter:
    pass


class Tileset:
    def __init__(self, tile_width: int, tile_height: int):
        self._tile_width = tile_width
        self._tile_height = tile_height

    @property
    def tile_width(self) -> int:
        return self._tile_width

    @property
    def tile_height(self) -> int:
        return self._tile_height

    def add_tile(self, tile: Tile):
        pass

    def get_tile(self, tile_id: int) -> Optional[Tile]:
        pass

    def load(self, src: Union[Readable, str]):
        pass


class TilemapLayer:
    def __init__(self, tilemap: 'Tilemap', name: str, row_count: int, col_count: int, position: Union[Vec2, Vec3, None], scroller: Union[Vec2, Vec3, None], flag: int):
        pass

    @property
    def name(self) -> str:
        return ''

    @property
    def flag(self) -> int:
        return 0

    @flag.setter
    def flag(self, flag):
        pass

    @property
    def tileset(self) -> Tileset:
        pass

    def get_tile(self, row: int, col: int) -> Optional[Tile]:
        pass

    def set_tile(self, row: int, col: int, tile: Union[int, RenderObject, Tile]):
        pass

    def copy_tiles(self, tiles: List[int], dest: Tuple[int, int, int, int]):
        pass


class Tilemap(Renderer):
    LAYER_FLAG_COLLIDABLE = 1
    LAYER_FLAG_SCROLLABLE = 2
    LAYER_FLAG_DEFAULT = 0

    def __init__(self, layer: LayerContext, w: int, h: int, tileset: Tileset):
        super().__init__()
        self._tileset = tileset

    def clear(self):
        pass

    @property
    def tileset(self) -> Tileset:
        return self._tileset

    @property
    def row_count(self):
        return 0

    @property
    def col_count(self):
        return 0

    @property
    def position(self):
        return None

    @position.setter
    def position(self, val):
        pass

    @property
    def scroller(self):
        return None

    @scroller.setter
    def scroller(self, val):
        pass

    @property
    def layers(self) -> List[TilemapLayer]:
        return []

    def add_layer(self, layer: TilemapLayer):
        pass

    def load(self, src: Union[Readable, str]):
        pass


class Function:
    def __init__(self, d):
        pass


class EventListener:
    def __init__(self, listener):
        pass

    def make_disposable(self, disposed: Optional[Boolean] = None) -> 'EventListener':
        pass

    def make_auto_release(self, ref_count: int = 1) -> 'EventListener':
        pass


class EventDispatcher(EventListener):
    def __init__(self):
        super().__init__(None)

    def on_key_event(self, code, on_press, on_release, on_click, on_repeat):
        pass

    def on_motion_event(self, on_down, on_up, on_click, on_move):
        pass

    def on_event(self, event):
        pass


class Glyph:
    def __init__(self, _type: Union[Integer, int], position: Optional[Vec3] = None, transform: Optional[Transform] = None, varyings: Optional['Varyings'] = None, visible: Optional['Visibility'] = None, disposed: Optional[Disposed] = None):
        pass

    @property
    def type(self) -> Optional[Integer]:
        return None

    @property
    def position(self) -> Optional[Vec3]:
        return None

    @property
    def transform(self) -> Optional[Transform]:
        return None


class Characters:
    def __init__(self, layer: Union[Layer, RenderLayer, LayerContext], text_scale=1.0, letter_spacing=0, line_height=0, line_indent=0):
        self._size = Size(0, 0)

    @property
    def text(self) -> str:
        return ''

    @text.setter
    def text(self, text: str):
        pass

    @property
    def contents(self) -> List[RenderObject]:
        return []

    @property
    def size(self) -> Size:
        return self._size

    def set_rich_text(self, text: str, **kwargs):
        pass


class StringBundle:

    def get_string(self, resid: str) -> str:
        return ''

    def get_string_array(self, resid: str) -> List[str]:
        return []


class Color(Vec4):
    def __init__(self, v):
        super().__init__(0, 0, 0, 0)

    def assign(self, other: 'Color'):
        pass

    @propertry
    def value(self) -> int:
        return 0


class Varyings:
    def __init__(self):
        pass

    def __setattr__(self, key, value):
        pass

    def __getattribute__(self, item):
        pass


class Visibility:
    def __init__(self, visible):
        self._visible = visible

    @property
    def visible(self):
        return self._visible

    @visible.setter
    def visible(self, v):
        self._visible = v


class Platform:
    @staticmethod
    def get_user_storage_path(p: str) -> str:
        return ''

    @staticmethod
    def get_external_storage_path(p: str) -> str:
        return ''

    @staticmethod
    def sys_call(cmd_id: int, cmd_value: str):
        pass


class RigidBody:

    def __init__(self):
        self._disposed = Boolean(False)

    def dispose(self):
        pass

    def bind(self, render_object: RenderObject):
        pass

    @property
    def id(self) -> int:
        return 0

    @property
    def type(self) -> int:
        return 0

    @property
    def xy(self) -> tuple:
        return 0, 0

    @property
    def xyz(self) -> tuple:
        return 0, 0, 0

    @property
    def position(self) -> Vec3:
        return Vec3(0, 0, 0)

    @property
    def size(self) -> Size:
        return Size(0, 0)

    @property
    def transform(self) -> Transform:
        pass

    @property
    def width(self) -> float:
        return 0

    @property
    def height(self) -> float:
        return 0

    @property
    def disposed(self) -> Boolean:
        return self._disposed

    @property
    def collision_callback(self):
        return None

    @collision_callback.setter
    def collision_callback(self, collision_callback):
        pass

    @property
    def render_object(self) -> RenderObject:
        return RenderObject(0)

    @property
    def tag(self):
        return None

    @tag.setter
    def tag(self, tag):
        pass


class SurfaceController:

    def add_renderer(self, renderer: Renderer):
        pass

    def add_control_layer(self, renderer: Renderer):
        pass

    def add_layer(self, renderer: Renderer):
        pass


class CollisionManifold:

    @property
    def contact_point(self) -> Tuple[float, float, float]:
        return 0, 0, 0

    @property
    def normal(self) -> Tuple[float, float, float]:
        return 0, 0, 0


class RayCastManifold:

    @property
    def distance(self) -> float:
        return 0

    @property
    def normal(self) -> Tuple[float, float, float]:
        return 0, 0, 0

    @property
    def rigid_body(self) -> Optional[RigidBody]:
        return None


class NarrowPhrase:

    def add_aabb_shape(self, shape_id: int, bounds: TYPE_RECT):
        pass

    def add_box_shape(self, shape_id: int, bounds: TYPE_RECT):
        pass


class Collider:

    BODY_SHAPE_AABB = -1
    BODY_SHAPE_BALL = -2
    BODY_SHAPE_BOX = -3
    BODY_SHAPE_CAPSULE = -4

    BODY_TYPE_KINEMATIC = 0
    BODY_TYPE_DYNAMIC = 1
    BODY_TYPE_STATIC = 2

    BODY_TYPE_SENSOR = 4

    BODY_FLAG_MANUAL_POSITION = 8
    BODY_FLAG_MANUAL_ROTATION = 16

    def create_body(self, type, shape, position, size=None, rotate=None) -> RigidBody:
        pass

    def ray_cast(self, ray_from, ray_to) -> List[RayCastManifold]:
        pass


class Command:

    def activate(self):
        pass

    def deactivate(self):
        pass


class CommandGroup:
    def __init__(self, mask: int, on_activate: Optional[Callable] = None, on_deactivate: Optional[Callable] = None):
        self._mask = 0

    @property
    def mask(self):
        return self._mask


class State:

    @property
    def active(self) -> bool:
        return False

    def link_command(self, command: Command):
        pass

    def link_command_group(self, command_group: CommandGroup):
        pass


class StateMachine:
    def __init__(self):
        pass

    def add_command(self, on_active: Optional[Callable] = None, command_group: CommandGroup = None) -> Command:
        pass

    def add_state(self, on_active: Optional[Callable] = None, fallback_state: Optional[State] = None) -> State:
        pass

    def start(self, entry: State):
        pass

    def transit(self, next_state: State):
        pass


class Texture:

    @property
    def width(self) -> int:
        return 0

    @property
    def height(self) -> int:
        return 0

    @property
    def depth(self) -> int:
        return 0

    @property
    def size(self) -> Size:
        return Size(0, 0)


class Emitter:

    @property
    def active(self):
        return False

    @active.setter
    def active(self, active):
        pass

    def activate(self):
        pass

    def deactivate(self):
        pass


class LayoutParam:
    DISPLAY_BLOCK = 0
    DISPLAY_FLOAT = 1
    DISPLAY_ABSOLUTE = 2

    GRAVITY_NONE = 0
    GRAVITY_LEFT = 1
    GRAVITY_RIGHT = 2
    GRAVITY_CENTER_HORIZONTAL = 3
    GRAVITY_TOP = 4
    GRAVITY_BOTTOM = 8
    GRAVITY_CENTER_VERTICAL = 12
    GRAVITY_CENTER = GRAVITY_CENTER_VERTICAL | GRAVITY_CENTER_HORIZONTAL
    GRAVITY_DEFAULT = GRAVITY_CENTER

    def __init__(self, size: Size, display: int = DISPLAY_BLOCK, gravity: int = GRAVITY_DEFAULT, weight: float = 0):
        pass


def __trace__():
    pass
