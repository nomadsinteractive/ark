"""

This module only servers as a mimic of the Ark runtime library.
Functionalities are limited or unimplemented.

Use it for:
1. Autocomplete
2. Documentation
3. Unit test(maybe)

"""
from typing import Callable, List, Type, TypeVar, Union, Optional, Dict, Tuple, Any

_BUILDABLE_TYPES = TypeVar('_BUILDABLE_TYPES', 'Arena', 'AudioPlayer', 'Boolean', 'Characters', 'Collider', 'Integer', 'ModelLoader', 'Numeric', 'NarrowPhrase',
                           'Layer', 'Vec2', 'Vec3', 'Vec4', 'Renderer', 'RenderLayer', 'RenderObject', 'Rotation', 'Size', 'StringBundle', 'Tilemap',
                           'TilemapImporter', 'Tileset', 'TilesetImporter', 'Transform', 'Varyings', 'View')


TYPE_INTEGER = Union[int, 'Integer']
TYPE_ENUM = Union[int, 'Enum']
TYPE_INT_OR_FLOAT = Union[int, float]
TYPE_NUMERIC = Union[TYPE_INT_OR_FLOAT, 'Numeric']
TYPE_RECT = tuple[TYPE_INT_OR_FLOAT, TYPE_INT_OR_FLOAT, TYPE_INT_OR_FLOAT, TYPE_INT_OR_FLOAT]
TYPE_VEC2 = Union[tuple[TYPE_NUMERIC, TYPE_NUMERIC], 'Vec2']
TYPE_VEC3 = Union[tuple[TYPE_NUMERIC, TYPE_NUMERIC, TYPE_NUMERIC], TYPE_VEC2, 'Vec3']
TYPE_RECTI = tuple[int, int, int, int]
TYPE_FLOAT4 = tuple[float, float, float, float]
TYPE_M4 = tuple[TYPE_FLOAT4, TYPE_FLOAT4, TYPE_FLOAT4, TYPE_FLOAT4]


def logd(*args):
    print(*args)


def logw(*args):
    print(*args)


def loge(*args):
    print(*args)


def set_trace_flag():
    pass


def load_asset(filename) -> str:
    with open(filename, 'rt') as fp:
        return fp.read()


def open_asset(filename: str):
    return load_asset(filename)


def load_asset_bundle(filepath: str) -> Optional['AssetBundle']:
    pass


def get_ref_manager():
    return None


def build_type() -> int:
    return 0


class Enum:
    BUILD_TYPE_UNDEFINED = 0
    BUILD_TYPE_DEBUG = 1
    BUILD_TYPE_RELEASE = 2
    BUILD_TYPE_RELEASE_WITH_DEBUG_INFO = 3
    BUILD_TYPE_PUBLISHED = 100

    RENDER_MODE_NONE = -1
    RENDER_MODE_LINES = 0
    RENDER_MODE_POINTS = 1
    RENDER_MODE_TRIANGLES = 2
    RENDER_MODE_TRIANGLE_STRIP = 3
    RENDER_MODE_COUNT = 4

    DRAW_PROCEDURE_AUTO = 0
    DRAW_PROCEDURE_DRAW_ARRAYS = 1
    DRAW_PROCEDURE_DRAW_ELEMENTS = 2
    DRAW_PROCEDURE_DRAW_INSTANCED = 3
    DRAW_PROCEDURE_DRAW_INSTANCED_INDIRECT = 4

    def __init__(self, value: int):
        pass

    def __int__(self) -> int:
        return 0


class Readable:
    pass


class Writable:
    pass


class ModelLoader:
    pass


class Asset:

    @property
    def location(self) -> str:
        return ''

    def read_string(self) -> str:
        pass


class AssetBundle:

    def get_asset(self, path: str) -> Asset:
        pass

    def get_bundle(self, path: str) -> 'AssetBundle':
        pass

    def list_assets(self, regex: str = '') -> list[Asset]:
        pass


class _Array:
    @property
    def native_ptr(self) -> int:
        return 0

    def wrap(self):
        pass

    def reset(self, other):
        pass

    def to_bytes(self) -> bytes:
        pass

    def to_byte_array(self) -> 'ByteArray':
        pass

    def __len__(self) -> int:
        return 0

    def __getitem__(self, item):
        pass

    def __setitem__(self, item, value):
        pass


class IntArray(_Array):
    def __init__(self, size: int, fill: int = 0):
        pass


class FloatArray(_Array):
    def __init__(self, size: int, fill: int = 0):
        pass


class ByteArray(_Array):
    def __init__(self, size: int, fill: int = 0):
        pass

    def to_integer(self) -> 'Integer':
        pass

    def to_numeric(self) -> 'Numeric':
        pass

    def to_vec2(self) -> 'Vec2':
        pass

    def to_vec3(self) -> 'Vec3':
        pass

    def to_vec4(self) -> 'Vec4':
        pass


class _Var:
    def __init__(self, val: Any):
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

    def clamp(self, a, b):
        pass

    def mod_floor(self, mod):
        pass

    def mod_ceil(self, mod):
        pass

    def floor(self):
        pass

    def ceil(self):
        pass

    def round(self):
        pass

    def if_else(self, condition, otherwise):
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

    def __mod__(self, other):
        pass

    def __gt__(self, other) -> 'Boolean':
        return Boolean(False)

    def __ge__(self, other) -> 'Boolean':
        return Boolean(False)

    def __lt__(self, other) -> 'Boolean':
        return Boolean(False)

    def __le__(self, other) -> 'Boolean':
        return Boolean(False)

    def __eq__(self, other) -> 'Boolean':
        return Boolean(False)

    def __ne__(self, other) -> 'Boolean':
        return Boolean(False)

    def __float__(self) -> float:
        return 0.0

    def __neg__(self):
        pass


class Json:
    def __init__(self):
        pass

    def dump(self) -> str:
        pass

    def to_bson(self) -> ByteArray:
        pass

    @staticmethod
    def from_bson(content: bytes | ByteArray) -> 'Json':
        pass


class DOMDocument:
    def __init__(self, name: str, value: str = '', type_: int = 0):
        pass

    @property
    def name(self) -> str:
        return ''

    @property
    def type(self) -> int:
        return 0

    @property
    def attributes(self) -> list:
        return []

    @property
    def children(self) -> list['DOMDocument']:
        return []

    def get_attribute(self, name: str):
        pass

    def get_child(self, name: str) -> 'DOMDocument':
        pass

    def ensure_child(self, name: str) -> 'DOMDocument':
        pass

    def add_child(self, doc: 'DOMDocument'):
        pass


class Manifest:
    def __init__(self, src: str, descriptor: Optional[DOMDocument] = None):
        self._src = src

    @property
    def src(self) -> str:
        return self._src


class IDHeap:
    def __init__(self, heap_size: int):
        pass

    def allocate(self, size: int, alignment: int = 1) -> int:
        pass

    def free(self, ptr: int):
        pass


class MessageLoop:

    def post(self, task: Callable[[], None], delay: float):
        pass

    def schedule(self, task: Callable[[], None], interval: float):
        pass


class Storage:

    def load(self, readable: Readable):
        pass

    def save(self, writable: Writable):
        pass

    def json_load(self, json: Json):
        pass

    def json_dump(self) -> Json:
        pass


class ApplicationBundle:

    def load_document(self, resid: str) -> DOMDocument:
        pass

    def load_json(self, resid: str) -> Json:
        pass

    def load_string(self, resid: str) -> str:
        pass


class Future:
    def __init__(self, canceled: Optional['Boolean'] = None, observer: Optional['Observer'] = None):
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


class Buffer:
    TYPE_VERTEX = 0
    TYPE_INDEX = 1
    TYPE_DRAW_INDIRECT = 2
    TYPE_STORAGE = 3
    TYPE_COUNT = 4

    USAGE_DYNAMIC = 0
    USAGE_STATIC = 1
    USAGE_COUNT = 2

    @property
    def size(self) -> int:
        return 0

    @property
    def id(self) -> int:
        return 0

    def upload(self, uploader: 'Uploader', future: Optional[Future] = None):
        pass

    def synchronize(self, offset: int, size: int, cancelled: 'Boolean') -> ByteArray:
        pass


class Texture:
    FORMAT_AUTO = 0x8000
    FORMAT_R = 0
    FORMAT_RG = 1
    FORMAT_RGB = 2
    FORMAT_RGBA = 3
    FORMAT_SIGNED = 8
    FORMAT_NORMALIZED = 16
    FORMAT_INTEGER = 32
    FORMAT_FLOAT = 64
    FORMAT_8_BIT = 128
    FORMAT_16_BIT = 256
    FORMAT_32_BIT = FORMAT_8_BIT | FORMAT_16_BIT

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
    def size(self) -> 'Size':
        return Size(0, 0)


class RenderController:
    US_ONCE = 0
    US_RELOAD = 1
    US_ON_SURFACE_READY = 2
    US_ONCE_AND_ON_SURFACE_READY = 3
    US_ON_CHANGE = 4
    US_ON_EVERY_FRAME = 8

    UPLOAD_PRIORITY_LOW = 0
    UPLOAD_PRIORITY_NORMAL = 1
    UPLOAD_PRIORITY_HIGH = 2

    def upload_buffer(self, buffer: Buffer, uploader: 'Uploader', upload_strategy: int, future: Optional[Future] = None,
                      upload_priority: int = UPLOAD_PRIORITY_NORMAL):
        pass

    def make_buffer(self, buffer_type: int, buffer_usage: int, uploader: Optional['Uploader'], upload_strategy: int, future: Optional[Future] = None) -> Buffer:
        pass

    def make_vertex_buffer(self, buffer_usage: int = Buffer.USAGE_DYNAMIC, uploader: Optional['Uploader'] = None) -> Buffer:
        pass

    def make_index_buffer(self, buffer_usage: int = Buffer.USAGE_DYNAMIC, uploader: Optional['Uploader'] = None) -> Buffer:
        pass

    def create_texture2d(self, bitmap: 'Bitmap', texture_format: int = Texture.FORMAT_AUTO, upload_strategy: int = US_ONCE_AND_ON_SURFACE_READY,
                         future: Optional[Future] = None) -> Texture:
        pass


class ApplicationFacade:

    def __init__(self):
        self._arena = None

    @property
    def clock(self) -> 'Clock':
        return Clock()

    @property
    def clock_interval(self) -> 'Numeric':
        return Numeric(0)

    @property
    def cursor_position(self) -> 'Vec2':
        return Vec2(0, 0)

    @property
    def surface_size(self) -> 'Size':
        return Size(0, 0)

    @property
    def application_controller(self) -> 'ApplicationController':
        return ApplicationController()

    @property
    def application_bundle(self) -> ApplicationBundle:
        return ApplicationBundle()

    @property
    def surface_controller(self) -> 'SurfaceController':
        return SurfaceController()

    @property
    def render_controller(self) -> RenderController:
        return RenderController()

    @property
    def camera(self) -> 'Camera':
        return Camera()

    @property
    def manifest(self) -> 'ApplicationManifest':
        return ApplicationManifest()

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

    def add_event_listener(self, event_listener: Callable[['Event'], bool], discarded: Optional['Boolean'] = None):
        pass

    def push_event_listener(self, event_listener: Callable[['Event'], bool], discarded: Optional['Boolean'] = None):
        pass

    def set_default_event_listener(self, event_listener: Callable[['Event'], bool]):
        pass

    def get_string(self, resid: str, def_value: Optional[str] = None) -> str:
        pass

    def get_string_array(self, resid: str) -> List[str]:
        pass

    def exit(self):
        pass

    def post(self, task: Callable, delay: float | list[float], canceled: Optional['Boolean'] = None):
        pass

    def schedule(self, task: Callable, interval: float, canceled: Optional['Boolean'] = None):
        pass

    def expect(self, condition: 'Boolean', observer: 'Observer', canceled: Optional['Boolean'] = None) -> Future:
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

    @property
    def texture(self) -> Texture:
        pass

    def has(self, c: int) -> bool:
        pass

    def get_item_uv(self, c: int) -> TYPE_RECT:
        pass

    def get_original_size(self, c:  int) -> tuple[float, float]:
        pass

    def get_pivot(self, c:  int) -> tuple[float, float]:
        pass

    def add_importer(self, importer: AtlasImporter, readable: Optional[Readable] = None):
        pass


class BroadPhrase:
    pass


class AudioPlayer:
    PLAY_OPTION_DEFAULT = -1
    PLAY_OPTION_LOOP_OFF = 0
    PLAY_OPTION_LOOP_ON = 1

    def play(self, source: Union[str, Readable], options: int = PLAY_OPTION_DEFAULT):
        pass


class Bitmap:
    def __init__(self, width: int, height: int, row_bytes: int, channels: int, data: Optional[ByteArray]):
        pass

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
    def bytes(self) -> bytes:
        return b''

    @property
    def byte_array(self) -> Optional[ByteArray]:
        return None


class ApplicationManifest:

    @property
    def name(self) -> str:
        return ''

    @property
    def renderer_resolution(self) -> 'Size':
        return Size(0, 0)


class Observer:
    def __init__(self, oneshot: bool = True):
        pass

    def update(self):
        pass

    def add_callback(self, callback: Callable):
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

    def reset(self, v: 'Renderer') -> Optional['Renderer']:
        pass

    def translate(self, position: Union[tuple, 'Vec2']) -> 'Renderer':
        return self

    def make_disposable(self, disposed: Union[bool, 'Boolean'] = None) -> 'Renderer':
        pass

    def make_visible(self, visibility: Union[bool, 'Boolean', 'Visibility']) -> 'Renderer':
        pass

    def make_auto_release(self, ref_count: int = 1) -> 'Renderer':
        pass


class RenderPass(Renderer):
    def __init__(self, shader: Shader, vertex_buffer: Buffer, index_buffer: Buffer, draw_count: 'Integer', render_mode: TYPE_ENUM, draw_procedure: TYPE_ENUM,
                 divided_uploaders: dict[int, 'Uploader']):
        super().__init__()


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
    def view(self) -> 'Mat4':
        pass

    @view.setter
    def view(self, view: 'Mat4'):
        pass

    @property
    def projection(self) -> 'Mat4':
        pass

    @projection.setter
    def projection(self, projection: 'Mat4'):
        pass

    @property
    def vp(self) -> 'Mat4':
        return None

    def ortho(self, left_top: TYPE_VEC2, right_bottom: TYPE_VEC2, clip: TYPE_VEC2):
        pass

    def frustum(self, left, right, top, bottom, near, far):
        pass

    def perspective(self, fov, aspect, near, far):
        pass

    def look_at(self, position: TYPE_VEC3, target: TYPE_VEC3, up: TYPE_VEC3):
        pass

    def to_world_position(self, screen_x: float, screen_y: float, z: float) -> tuple:
        pass

    def to_viewport_position(self, position: TYPE_VEC3) -> 'Vec3':
        pass


class String:
    def __init__(self, value):
        pass

    @property
    def val(self) -> str:
        return ''

    def set(self, val: str):
        pass

    def if_else(self, condition: 'Boolean', negative: Union[str, 'String']) -> 'String':
        pass

    @staticmethod
    def format(msg: str, **kwargs) -> 'String':
        pass


class Boolean(_Var):
    def __init__(self, value):
        _Var.__init__(self, value)

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

    def sod(self, d0: float, k: float, z: float = 1.0, r: float = 0, t: Optional['Numeric'] = None) -> 'Numeric':
        pass

    @staticmethod
    def vibrate(s0: float, v0: float, s1: float, v1: float, duration: float, t=None):
        return None

    def __floordiv__(self, other):
        pass


class Integer(_Var):
    REPEAT_NONE = 0
    REPEAT_REVERSE = 1
    REPEAT_LOOP = 4
    REPEAT_LAST = 8
    REPEAT_NOTIFY = 16

    def __init__(self, value: Union[int, 'Integer', Numeric, list['Integer']]):
        super().__init__(value)

    @staticmethod
    def repeat(array: List[int], repeat: int) -> 'ExpectationI':
        pass

    @staticmethod
    def to_repeat(repeat: str) -> int:
        pass

    def animate(self, interval: Union[Numeric, float] = None, duration: Union[Numeric, float] = None) -> 'Integer':
        pass

    def __len__(self):
        pass

    def __getitem__(self, index: 'Integer') -> 'Integer':
        pass


class Expendable(Boolean):
    def __init__(self, disposed=False):
        pass

    def dispose(self):
        pass

    def observe(self, observer: Observer) -> Boolean:
        return Boolean(False)

    def __bool__(self):
        return False


class Vec2(_Var):
    def __init__(self, x, y):
        self._x = x
        self._y = y

    @property
    def x(self) -> Numeric:
        return self._x

    @x.setter
    def x(self, v):
        pass

    @property
    def y(self) -> Numeric:
        return self._y

    @property
    def xy(self) -> 'Vec2':
        return self

    @xy.setter
    def xy(self, val):
        pass

    @y.setter
    def y(self, v):
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

    def integral_with_resistance(self, v0: tuple[float, float], cd: Union[float, 'Numeric'], t: Optional['Numeric'] = None) -> 'Vec2':
        pass

    def wrap(self) -> 'Vec2':
        return Vec2(self._x, self._y)

    def synchronize(self, disposed: Optional[Boolean] = None) -> 'Vec2':
        return Vec2(self._x, self._y)

    def attract(self, s0: Union[tuple, 'Vec2'], duration: float, t: Optional[Numeric] = None) -> 'Vec2':
        pass

    def fence(self, plane: Union['Vec3', tuple], observer: Union[Observer, Callable[[], None]]) -> 'Vec2':
        return Vec2(self._x, self._y)

    def atan2(self) -> Numeric:
        pass

    def normalize(self):
        pass

    def distance_to(self, other) -> Numeric:
        pass

    def hypot(self) -> Numeric:
        pass

    def to_size(self) -> 'Size':
        pass

    def sod(self, d0, k: float, z: float, r: float, t: Optional['Numeric'] = None):
        pass

    def extend(self, v):
        pass

    def __iter__(self):
        pass

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
    def xyz(self) -> 'Vec3':
        return self

    @xyz.setter
    def xyz(self, v):
        pass

    def cross(self, other) -> 'Vec3':
        pass

    def extend(self, w: Union[float, Numeric]) -> 'Vec4':
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
    def xyzw(self) -> tuple[float, float, float, float]:
        return 0, 0, 0, 0

    @property
    def vw(self):
        return self._z

    @vw.setter
    def vw(self, v):
        pass


class _Mat:
    def __mul__(self, other):
        pass

    def __matmul__(self, other):
        pass

    @property
    def val(self) -> tuple:
        return 0, 0, 0, 0

    def translate(self, translation: Vec2 | Vec3):
        pass

    def rotate(self, rot: Union[float, Numeric, 'Rotation']):
        pass

    def freeze(self):
        pass

    def if_else(self, condition: Boolean, other):
        pass

    @staticmethod
    def identity():
        pass


class Mat2(_Mat):
    def __init__(self, t=None, b=None):
        pass


class Mat3(_Mat):
    def __init__(self, t=None, b=None, n=None):
        pass


class Mat4(_Mat):
    def __init__(self, t=None, b=None, n=None, w=None):
        pass


class Uploader:
    def __init__(self, delegate: Union[Numeric, Vec2, Vec3, Vec4, list[_Mat], list[tuple], list[int], list['Uploader'], Any], size: int = 0):
        pass

    @property
    def size(self):
        return 0

    def reserve(self, size: int) -> 'Uploader':
        pass

    def remap(self, size: int, offset: int = 0) -> 'Uploader':
        pass

    def add_input(self, offset: int, input_: 'Uploader'):
        pass

    def remove_input(self, offset: int):
        pass

    def wrap(self) -> 'Uploader':
        pass

    def reset(self, delegate: 'Uploader'):
        pass

    def mark_dirty(self):
        pass

    @staticmethod
    def blank(size: int, fill: int = 0) -> 'Uploader':
        pass


class AnimationInput(Uploader):

    def get_node_matrix(self, node_name: str) -> Mat4:
        pass

    def get_transform_variance(self, c: tuple[float, float, float], nodes: list[str]) -> list[float]:
        pass


class Animation:
    @property
    def ticks(self) -> float:
        return 0

    @property
    def tps(self) -> float:
        return 0

    @property
    def duration(self) -> float:
        return 0

    @property
    def node_names(self) -> List[str]:
        return []

    def make_input(self, t: Numeric, on_complete: Optional[Callable] = None) -> AnimationInput:
        pass


class MaterialTexture:

    @property
    def color(self) -> Optional[Vec4]:
        return None

    @property
    def bitmap(self) -> Optional[Bitmap]:
        return None


class Material:
    @property
    def id(self) -> int:
        return 0

    @property
    def name(self) -> str:
        return ''

    @property
    def base_color(self) -> Optional[MaterialTexture]:
        return None

    @property
    def normal(self) -> Optional[MaterialTexture]:
        return None

    @property
    def roughness(self) -> Optional[MaterialTexture]:
        return None

    @property
    def metallic(self) -> Optional[MaterialTexture]:
        return None

    @property
    def specular(self) -> Optional[MaterialTexture]:
        return None


class Mesh:
    @property
    def name(self) -> str:
        return ''

    @property
    def vertex_count(self) -> int:
        return 0

    @property
    def indices(self) -> list[int]:
        return []

    @property
    def vertices(self) -> list[float]:
        return []

    @property
    def material(self) -> Optional[Material]:
        return None


class Node:

    @property
    def name(self) -> str:
        return ''

    @property
    def child_nodes(self) -> list['Node']:
        return []

    @property
    def meshes(self) -> list[Mesh]:
        return []

    @property
    def transform(self) -> TYPE_M4:
        return ((0,) * 4, ) * 4

    def find_child_node(self, name: str) -> Optional['Node']:
        pass


class Metrics:

    @property
    def width(self) -> float:
        return 0

    @property
    def height(self) -> float:
        return 0

    @property
    def depth(self) -> float:
        return 0

    @property
    def aabb_min(self) -> tuple[float, float, float]:
        return 0, 0, 0

    @property
    def aabb_max(self) -> tuple[float, float, float]:
        return 0, 0, 0

    @property
    def size(self) -> tuple[float, float, float]:
        return 0, 0, 0


class Model:

    @property
    def bounds(self) -> Metrics:
        return Metrics()

    @property
    def occupies(self) -> Metrics:
        return Metrics()

    @property
    def materials(self) -> list[Material]:
        return []

    @property
    def meshes(self) -> list[Mesh]:
        return []

    @property
    def index_count(self) -> int:
        return 0

    @property
    def vertex_count(self) -> int:
        return 0

    @property
    def animations(self) -> dict[str, Animation]:
        return {}

    @property
    def node_names(self) -> list[str]:
        return []

    @property
    def root_node(self) -> Optional[Node]:
        return None

    @property
    def indices(self) -> Uploader:
        return Uploader(None)

    @property
    def vertices(self):
        pass

    def find_node(self, name: str) -> Optional[Node]:
        pass

    def get_animation(self, name: str) -> Animation:
        pass


class MaterialBundle:
    def __init__(self, materials: list[Material] = None):
        pass

    def add_material(self, name: str, material: Material):
        pass


class ModelBundle:

    @property
    def index_length(self) -> int:
        return 0

    @property
    def vertex_length(self) -> int:
        return 0

    def get_model(self, t: int) -> Model:
        pass

    def import_model(self, t: int, manifest: str | Manifest, future: Optional[Future] = None):
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

    def make_layer(self, model_loader: Optional[ModelLoader] = None, position: Optional[Vec3] = None, visible: Optional[Boolean] = None,
                   disposable: Optional[Boolean] = None) -> 'Layer':
        pass


class Layer:
    TYPE_UNSPECIFIED = 0
    TYPE_DYNAMIC = 1
    TYPE_STATIC = 2

    def __init__(self, layer_context: Optional[LayerContext] = None):
        super().__init__()

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

    def add_view(self, view: 'View', disposable: Optional[Boolean] = None):
        pass

    def add_layer(self, layer: Renderer):
        pass

    def add_render_layer(self, render_layer: Renderer):
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

    def timeout(self, seconds: float, timeout_value: bool = True) -> 'Boolean':
        pass

    def pause(self):
        pass

    def resume(self):
        pass


class Event:
    ACTION_NONE = 0
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
    CODE_KEYBOARD_0 = ord('0')
    CODE_KEYBOARD_1 = ord('1')
    CODE_KEYBOARD_2 = ord('2')
    CODE_KEYBOARD_3 = ord('3')
    CODE_KEYBOARD_4 = ord('4')
    CODE_KEYBOARD_5 = ord('5')
    CODE_KEYBOARD_6 = ord('6')
    CODE_KEYBOARD_7 = ord('7')
    CODE_KEYBOARD_8 = ord('8')
    CODE_KEYBOARD_9 = ord('9')
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

    BUTTON_MOUSE_LEFT = 0
    BUTTON_MOUSE_MIDDLE = 1
    BUTTON_MOUSE_RIGHT = 2
    BUTTON_MOTION_POINTER1 = 3
    BUTTON_MOTION_POINTER2 = 4
    BUTTON_MOTION_POINTER3 = 5
    BUTTON_MOTION_POINTER4 = 6

    @property
    def action(self) -> int:
        return self.ACTION_KEY_DOWN

    @property
    def code(self) -> int:
        return self.CODE_NONE

    @property
    def button(self) -> int:
        return self.BUTTON_MOUSE_LEFT

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


class Rotation(Vec4):
    def __init__(self, theta: Union[float, Numeric], axis: Union[Vec3, tuple] = None):
        super().__init__(0, 0, 0, 0)
        self._theta = theta
        self._axis = axis


    def reset(self, quaternion: Vec4):
        pass
    @property
    def theta(self) -> Numeric:
        return self._theta

    @theta.setter
    def theta(self, theta: float | Numeric):
        pass

    @property
    def axis(self) -> Vec3:
        return self._axis

    def set_rotation(self, theta: Union[float, Numeric], axis: Union[Vec3, tuple]):
        pass

    def set_euler(self, pitch: Union[float, Numeric], yaw: Union[float, Numeric], roll: Union[float, Numeric]):
        pass


class Transform(Mat4):
    TYPE_LINEAR_2D = 0
    TYPE_LINEAR_3D = 1

    def __init__(self, t: int = TYPE_LINEAR_3D, rotation: Optional[Rotation] = None, scale: Vec3 | Vec2 | None = None, translation: Vec3 | Vec2 | None = None):
        self._type = t
        self._rotation = rotation
        self._scale = scale
        self._translation = translation

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
    def translation(self) -> Vec3:
        return self._translation

    @translation.setter
    def translation(self, v):
        self._translation = v


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
    def acos(x):
        pass

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
        pass

    @staticmethod
    def bezier(p0, p1, p2, t):
        pass

    @staticmethod
    def quadratic(a, b, c):
        return 0, 0

    @staticmethod
    def projectile(dx, dy, v, g, sid):
        return 0, 0


class Size(Vec3):
    def __init__(self, width: Union[float, Numeric], height: Union[float, Numeric], depth: Union[float, Numeric, None] = None):
        super().__init__(width, height, depth)

    @property
    def width(self) -> Numeric:
        return self.x

    @width.setter
    def width(self, width: Union[float, Numeric]):
        pass

    @property
    def height(self) -> Numeric:
        return self.y

    @height.setter
    def height(self, height: Union[float, Numeric]):
        pass

    @property
    def depth(self) -> Numeric:
        return self.z

    @depth.setter
    def depth(self, depth: Union[float, Numeric]):
        pass


class Tile:
    def __init__(self, id_: int, type_: str = '', shape_id: int = -1, render_object: Optional[RenderObject] = None):
        pass

    @property
    def id(self) -> int:
        return 0

    @property
    def type(self) -> str:
        return ''

    @type.setter
    def type(self, t: str):
        pass

    @property
    def shape_id(self) -> int:
        return 0

    @shape_id.setter
    def shape_id(self, shape_id: int):
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
    def __init__(self, tile_size: Size):
        self._tile_size = tile_size

    @property
    def tiles(self) -> Dict[int, Tile]:
        return {}

    @property
    def tile_width(self) -> float:
        return self._tile_size.width

    @property
    def tile_height(self) -> float:
        return self._tile_size.height

    def add_tile(self, tile: Tile):
        pass

    def get_tile(self, tile_id: int) -> Optional[Tile]:
        pass

    def load(self, src: Union[Readable, str]):
        pass


class TilemapLayer(Renderer):
    def __init__(self, tileset: Tileset, name: str, row_count: int, col_count: int, position: Optional[TYPE_VEC3] = None, visible: Optional[Boolean] = None,
                 collision_filter: Optional['CollisionFilter'] = None):
        super().__init__()

    @property
    def position(self) -> Optional[Vec3]:
        return None

    @position.setter
    def position(self, position: Optional[TYPE_VEC3]):
        pass

    @property
    def col_count(self) -> int:
        return 0

    @property
    def row_count(self) -> int:
        return 0

    @property
    def name(self) -> str:
        return ''

    @property
    def flags(self) -> int:
        return 0

    @flags.setter
    def flags(self, flag):
        pass

    @property
    def tileset(self) -> Tileset:
        pass

    def get_tile(self, col: int, row: int) -> Optional[Tile]:
        pass

    def get_tile_rect(self, rect: TYPE_RECTI) -> list[int]:
        pass

    def set_tile(self, col: int, row: int, tile: Union[int, RenderObject, Tile, None]):
        pass

    def set_tile_rect(self, tiles: list[int], dest: TYPE_RECTI):
        pass

    def foreach_tile(self, callback: Callable[[int, int, Tile], bool]):
        pass

    def resize(self, col_count: int, row_count: int, offset_x: int = 0, offset_y: int = 0):
        pass

    def clear(self):
        pass


class Tilemap:
    LAYER_FLAG_DEFAULT = 0
    LAYER_FLAG_COLLIDABLE = 1
    LAYER_FLAG_SCROLLABLE = 2
    LAYER_FLAG_INVISIBLE = 4

    def __init__(self, tileset: Tileset, render_layer: Optional[RenderLayer] = None):
        super().__init__()
        self._tileset = tileset

    @property
    def tileset(self) -> Tileset:
        return self._tileset

    @property
    def position(self):
        return None

    @position.setter
    def position(self, val):
        pass

    @property
    def layers(self) -> List[TilemapLayer]:
        return []

    @property
    def storage(self) -> Optional[Storage]:
        return None

    def clear(self):
        pass

    def make_layer(self, name: str, col_count: int, row_count: int, position: Optional[TYPE_VEC3] = None, visible: Optional[Boolean] = None,
                   collision_filter: Optional['CollisionFilter'] = None, zorder: float = 0) -> TilemapLayer:
        pass

    def add_layer(self, layer: TilemapLayer, zorder: float = 0):
        pass

    def remove_layer(self, layer: TilemapLayer):
        pass

    def load(self, src: Union[Readable, str]):
        pass

    def find_route(self, start: tuple[int, int], goal: tuple[int, int]) -> list[tuple[int, int]]:
        pass


class Function:
    def __init__(self, d):
        pass


class EventListener:
    def __init__(self, listener: Any = None):
        pass

    def on_event(self, event: Event):
        pass

    def reset(self, listener: Any = None):
        pass

    def add_event_listener(self, listener: Any, disposed: Optional[Boolean] = None, push_front: bool = False):
        pass


class EventDispatcher(EventListener):
    def __init__(self):
        super().__init__(None)

    def on_event(self, event):
        pass

    def on_key_event(self, code, on_press, on_release, on_repeat):
        pass

    def on_motion_event(self, on_down, on_up, on_click, on_move):
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


class Text:
    def __init__(self, render_layer: RenderLayer, text: String | str | None = None, glyph_maker: Any = None, text_scale: float = 1.0, letter_spacing: float = 0,
                 line_height: float = 0, line_indent: float = 0):
        pass

    @property
    def text(self) -> str:
        return ''

    @text.setter
    def text(self, text: str):
        pass

    @property
    def size(self) -> Size:
        return Size(0, 0)

    @property
    def contents(self) -> list[RenderObject]:
        return []

    @property
    def position(self) -> Vec3:
        return Vec3(0, 0, 0)

    @position.setter
    def position(self, position):
        pass

    @property
    def transform(self) -> Optional[Mat4]:
        pass

    @transform.setter
    def transform(self, transform: Optional[Mat4]):
        pass

    def show(self, discarded: Optional[Boolean] = None):
        pass

    def set_rich_text(self, text: str, **kwargs):
        pass


class StringBundle:

    def get_string(self, resid: str) -> str:
        return ''

    def get_string_array(self, resid: str) -> List[str]:
        return []


class Color(Vec4):
    def __init__(self, r, g, b, a):
        super().__init__(r, g, b, a)

    @property
    def value(self) -> int:
        return 0

    def to_vec3(self) -> Vec3:
        pass


class Varyings:
    def __init__(self):
        pass

    def __setattr__(self, key: str, value):
        pass

    def __getattr__(self, key: str) -> Any:
        pass

    def __getitem__(self, subkey: str) -> 'Varyings':
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

    def show(self):
        pass

    def hide(self):
        pass


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


class Shape:

    SHAPE_ID_NONE = 0
    SHAPE_ID_AABB = -1
    SHAPE_ID_BALL = -2
    SHAPE_ID_BOX = -3
    SHAPE_ID_CAPSULE = -4

    def __init__(self, _id: int, size: Optional[Vec3] = None):
        pass

    @property
    def id(self) -> int:
        return 0

    @property
    def size(self) -> Optional[Vec3]:
        return None


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
    def meta_id(self) -> int:
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
    def collision_filter(self) -> Optional['CollisionFilter']:
        return None

    @collision_filter.setter
    def collision_filter(self, collision_filter: Optional['CollisionFilter']):
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

    def add_renderer(self, renderer: Renderer, discarded: Optional[Boolean] = None, visible: Optional[Boolean] = None):
        pass

    def add_control_layer(self, renderer: Renderer, discarded: Optional[Boolean] = None, visible: Optional[Boolean] = None):
        pass

    def add_layer(self, renderer: Renderer, discarded: Optional[Boolean] = None, visible: Optional[Boolean] = None):
        pass


class CollisionFilter:
    def __init__(self, category_bits: int = 1, mask_bits: int = 0xffffffff, group_index: int = 0):
        pass

    @property
    def category_bits(self) -> int:
        return 0

    @property
    def mask_bits(self) -> int:
        return 0

    @property
    def group_index(self) -> int:
        return 0


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
    pass


class Collider:

    BODY_SHAPE_AABB = -1
    BODY_SHAPE_BALL = -2
    BODY_SHAPE_BOX = -3
    BODY_SHAPE_CAPSULE = -4

    SHAPE_TYPE_NONE = "ark_shape_type_none"
    SHAPE_TYPE_AABB = "ark_shape_type_aabb"
    SHAPE_TYPE_BALL = "ark_shape_type_ball"
    SHAPE_TYPE_BOX = "ark_shape_type_box"
    SHAPE_TYPE_CAPSULE = "ark_shape_type_capsule"

    BODY_TYPE_KINEMATIC = 0
    BODY_TYPE_DYNAMIC = 1
    BODY_TYPE_STATIC = 2

    BODY_TYPE_SENSOR = 4

    BODY_FLAG_MANUAL_POSITION = 8
    BODY_FLAG_MANUAL_ROTATION = 16

    def create_body(self, type_: int | Integer, shape: Optional[Shape], position, rotate=None, disposed: Optional[Boolean] = None) -> RigidBody:
        pass

    def ray_cast(self, ray_from, ray_to, collision_filter: Optional[CollisionFilter] = None) -> List[RayCastManifold]:
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

    def add_command(self, on_activate: Optional[Callable] = None, on_deactivate: Optional[Callable] = None, command_group: CommandGroup = None) -> Command:
        pass

    def add_state(self, on_active: Optional[Callable] = None, fallback_state: Optional[State] = None) -> State:
        pass

    def start(self, entry: State):
        pass

    def transit(self, next_state: State):
        pass


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

    def __init__(self, size: Size = None, display: int = DISPLAY_BLOCK, gravity: int = GRAVITY_DEFAULT, weight: float = 0):
        pass


class View:

    @property
    def discarded(self) -> Boolean:
        return Boolean(False)

    @property
    def position(self) -> Vec3:
        return Vec3(0, 0, 0)

    @property
    def layout_param(self) -> LayoutParam:
        return LayoutParam()

    def add_view(self, view: 'View'):
        pass


class Graph:
    def __init__(self):
        pass

    @property
    def nodes(self) -> set['GraphNode']:
        return set()

    def make_node(self, position: Vec3, tag: Any = None) -> 'GraphNode':
        pass


class GraphNode:

    @property
    def position(self) -> tuple[float, float, float]:
        return 0, 0, 0

    @property
    def tag(self) -> Any:
        return None

    @tag.setter
    def tag(self, tag: Any):
        pass

    def add_route(self, to_node: 'GraphNode', length: float):
        pass

    def find_route(self, goal: 'GraphNode') -> list['GraphNode']:
        pass


class ConvexHullResolver:
    def __init__(self, check_error: bool = True):
        pass

    def resolve(self, points: Optional[list[tuple[float, float]]]) -> list[tuple[float, float]]:
        pass


class PrimitiveModelFactory:
    def __init__(self, mat4: Optional[Mat4] = None):
        pass

    def make_triangle(self, tex_coordinate: Optional[tuple[float, float, float, float]] = None) -> Model:
        pass

    def make_plane(self, cols: int, rows: int, tex_coordinate: Optional[tuple[float, float, float, float]] = None) -> Model:
        pass


class Entity:
    def __init__(self, *components):
        pass

    @property
    def id(self) -> int:
        return 0

    def dispose(self):
        pass


class WithLayer:
    def __init__(self, layer: RenderLayer | Layer):
        pass


class Tag:
    def __init__(self, info: Any):
        pass

    @property
    def info(self) -> Any:
        return None


class CollisionCallback:
    def __init__(self, callback_obj):
        pass


class Label:
    def __init__(self, text: Text, layout_param: Optional[LayoutParam] = None):
        pass


def __trace__():
    pass
