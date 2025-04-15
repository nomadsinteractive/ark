"""

This module only servers as a mimic of the Ark runtime library.
Functionalities are limited or unimplemented.

Use it for:
1. Autocomplete
2. Documentation
3. Unit test(maybe)

"""
from typing import Callable, List, Type, TypeVar, Union, Optional, Dict, Tuple, Any, Self

_BUILDABLE_TYPES = TypeVar('_BUILDABLE_TYPES', 'Arena', 'AudioPlayer', "Boolean", 'Characters', 'Collider', 'Integer', 'ModelLoader', 'Numeric', 'NarrowPhrase',
                           'Layer', 'Vec2', 'Vec3', 'Vec4', 'Renderer', 'RenderLayer', 'RenderObject', 'Rotation', 'Size', 'StringBundle', 'Tilemap',
                           'TilemapImporter', 'Tileset', 'TilesetImporter', 'Transform', 'Varyings', 'View')


TYPE_INTEGER = Union[int, "Integer"]
TYPE_BOOLEAN = Union[bool, "Boolean"]
TYPE_ENUM = Union[int, 'Enum']
TYPE_INT_OR_FLOAT = Union[int, float]
TYPE_NUMERIC = Union[TYPE_INT_OR_FLOAT, 'Numeric']
TYPE_RECT = tuple[TYPE_INT_OR_FLOAT, TYPE_INT_OR_FLOAT, TYPE_INT_OR_FLOAT, TYPE_INT_OR_FLOAT]
TYPE_VEC2 = Union[tuple[TYPE_NUMERIC, TYPE_NUMERIC], 'Vec2']
TYPE_VEC3 = Union[tuple[TYPE_NUMERIC, TYPE_NUMERIC, TYPE_NUMERIC], 'Vec3']
TYPE_VEC4 = Union[tuple[TYPE_NUMERIC, TYPE_NUMERIC, TYPE_NUMERIC, TYPE_NUMERIC],'Vec4']
TYPE_RECTI = tuple[int, int, int, int]
TYPE_FLOAT2 = tuple[float, float]
TYPE_FLOAT3 = tuple[float, float, float]
TYPE_FLOAT4 = tuple[float, float, float, float]
TYPE_M4 = tuple[TYPE_FLOAT4, TYPE_FLOAT4, TYPE_FLOAT4, TYPE_FLOAT4]
TYPE_NAMED_HASH = Union[int, str]
TYPE_RUNNABLE = Union["Runnable", Callable[[], None]]


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


def is_ndc_y_up() -> bool:
    return True


def is_ark_type(cls: type) -> bool:
    return False


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

    UPLOAD_STRATEGY_ONCE = 0
    UPLOAD_STRATEGY_RELOAD = 1
    UPLOAD_STRATEGY_ON_SURFACE_READY = 2
    UPLOAD_STRATEGY_ONCE_AND_ON_SURFACE_READY = 3
    UPLOAD_STRATEGY_ON_CHANGE = 4
    UPLOAD_STRATEGY_ON_EVERY_FRAME = 8

    UPLOAD_PRIORITY_LOW = 0
    UPLOAD_PRIORITY_NORMAL = 1
    UPLOAD_PRIORITY_HIGH = 2

    def __init__(self, value: int):
        pass

    def __int__(self) -> int:
        return 0


class NamedHash:
    def __init__(self, value: TYPE_NAMED_HASH):
        pass

    @property
    def hash(self) -> int:
        return 0

    @property
    def name(self) -> str:
        return ''

    @staticmethod
    def reverse(hash: int) -> str:
        pass

    def __index__(self) -> int:
        pass


class Readable:
    pass


class Runnable:

    def __call__(self):
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

    def intertwine(self, components: list[Self]) -> Self:
        pass

    def to_bytes(self) -> bytes:
        pass

    def to_byte_array(self) -> "ByteArray":
        pass

    def __len__(self) -> int:
        return 0

    def __getitem__(self, item) -> Self:
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

    def to_integer(self) -> "Integer":
        pass

    def to_numeric(self) -> "Numeric":
        pass

    def to_vec2(self) -> "Vec2":
        pass

    def to_vec3(self) -> "Vec3":
        pass

    def to_vec4(self) -> "Vec4":
        pass
    
    def to_mat2(self) -> "Mat2":
        pass

    def to_mat3(self) -> "Mat3":
        pass

    def to_mat4(self) -> "Mat4":
        pass

    def __getitem__(self, item: slice) -> Self:
        pass


class _Var:

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

    def __gt__(self, other) -> "Boolean":
        return Boolean(False)

    def __ge__(self, other) -> "Boolean":
        return Boolean(False)

    def __lt__(self, other) -> "Boolean":
        return Boolean(False)

    def __le__(self, other) -> "Boolean":
        return Boolean(False)

    def __eq__(self, other) -> "Boolean":
        return Boolean(False)

    def __ne__(self, other) -> "Boolean":
        return Boolean(False)

    def __float__(self) -> float:
        return 0.0

    def __neg__(self):
        pass


class Boolean(_Var):
    def __init__(self, value):
        pass

    def toggle(self):
        pass

    def negative(self) -> "Boolean":
        pass

    def __or__(self, other) -> "Boolean":
        pass

    def __and__(self, other) -> "Boolean":
        pass


class Behavior:
    def __init__(self, delegate: Any):
        pass

    def __getitem__(self, name: str) -> Runnable:
        pass

    def create_runnable(self, name: str) -> Optional[Runnable]:
        pass

    def create_collision_callback(self, on_begin_contact:str = 'on_begin_contact', on_end_contact: str = 'on_end_contact') -> "CollisionCallback":
        pass

    def create_event_listener(self, on_event:str = 'on_event') -> "EventListener":
        pass


class Crate:
    def __init__(self):
        pass

    def set_component(self, type_id: int, component: Any):
        pass

    def get_component(self, type_id: int) -> Any:
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
    def __init__(self, canceled: Optional[Boolean] = None, observer: Optional[Runnable] = None, count_down: int = 1):
        pass

    @property
    def is_canceled(self) -> Boolean:
        pass

    @property
    def is_done(self) -> Boolean:
        pass

    @property
    def is_done_or_canceled(self) -> Boolean:
        pass

    def cancel(self):
        pass

    def notify(self):
        pass



class Ref:

    def __init__(self, refid: int):
        pass

    def __index__(self):
        pass

    @property
    def id(self) -> int:
        pass

    @property
    def discarded(self) -> Optional[Boolean]:
        pass

    @discarded.setter
    def discarded(self, discarded: Boolean):
        pass

    def discard(self):
        pass

    def to_entity(self) -> "Entity":
        pass

    def to_rigidbody(self) -> "Rigidbody":
        pass


class Buffer:

    USAGE_BIT_VERTEX = 1
    USAGE_BIT_INDEX = 2
    USAGE_BIT_DRAW_INDIRECT = 4
    USAGE_BIT_STORAGE = 8
    USAGE_BIT_DYNAMIC = 16
    USAGE_BIT_TRANSFER_SRC = 32
    USAGE_BIT_HOST_VISIBLE = 64

    def __init__(self, usage_bits: int, uploader: "Uploader"):
        pass

    @property
    def size(self) -> int:
        return 0

    @property
    def id(self) -> int:
        return 0

    def upload(self, uploader: 'Uploader', future: Optional[Future] = None):
        pass

    def synchronize(self, offset: int, size: int, cancelled: "Boolean") -> ByteArray:
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

    def __init__(self, bitmap: "Bitmap", texture_format: int = FORMAT_AUTO, upload_strategy: int = Enum.UPLOAD_STRATEGY_ONCE_AND_ON_SURFACE_READY, future: Optional[Future] = None):
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
    def size(self) -> 'Size':
        return Size(0, 0)

    def reset(self, texture: "Texture"):
        pass


class RenderController:

    def upload_buffer(self, buffer: Buffer, uploader: 'Uploader', upload_strategy: int, future: Optional[Future] = None,
                      upload_priority: int = Enum.UPLOAD_PRIORITY_NORMAL):
        pass

    def make_buffer(self, buffer_type: int, buffer_usage: int, uploader: Optional['Uploader'], upload_strategy: int, future: Optional[Future] = None) -> Buffer:
        pass

    def make_vertex_buffer(self, buffer_usage: int = Buffer.USAGE_BIT_DYNAMIC, uploader: Optional['Uploader'] = None) -> Buffer:
        pass

    def make_index_buffer(self, buffer_usage: int = Buffer.USAGE_BIT_DYNAMIC, uploader: Optional['Uploader'] = None) -> Buffer:
        pass

    def create_texture2d(self, bitmap: 'Bitmap', texture_format: int = Texture.FORMAT_AUTO, upload_strategy: int = Enum.UPLOAD_STRATEGY_ONCE_AND_ON_SURFACE_READY,
                         future: Optional[Future] = None) -> Texture:
        pass


class ApplicationFacade:

    def __init__(self):
        self._activity = None

    @property
    def clock(self) -> "Clock":
        return Clock()

    @property
    def clock_interval(self) -> "Numeric":
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
    def activity(self) -> "Activity":
        return self._activity

    @activity.setter
    def activity(self, activity: "Activity"):
        self._activity = activity

    @property
    def argv(self) -> List[str]:
        return []

    def to_frag_coord(self, xy: 'Vec2', resolution: Optional['Size'] = None) -> 'Vec2':
        pass

    def add_string_bundle(self, name: str, string_bundle: 'StringBundle'):
        pass

    def create_resource_loader(self, name: str, **kwargs) -> 'ResourceLoader':
        pass

    def make_message_loop(self, clock: 'Clock') -> MessageLoop:
        pass

    def add_pre_render_task(self, task: Callable, expired: "Boolean" = None):
        pass

    def add_control_layer(self, control: Union['Layer', 'Renderer']):
        pass

    def add_event_listener(self, event_listener: Callable[["Event"], bool], discarded: Optional["Boolean"] = None):
        pass

    def push_event_listener(self, event_listener: Callable[["Event"], bool], discarded: Optional["Boolean"] = None):
        pass

    def set_default_event_listener(self, event_listener: Callable[["Event"], bool]):
        pass

    def get_string(self, resid: str, def_value: Optional[str] = None) -> str:
        pass

    def get_string_array(self, resid: str) -> List[str]:
        pass

    def exit(self):
        pass

    def post(self, task: Optional[Callable], delay: float | list[float], canceled: Optional["Boolean"] = None) -> Future:
        pass

    def schedule(self, task: Optional[Callable], interval: float, canceled: Optional["Boolean"] = None, count_down: int = 0) -> Future:
        pass

    def expect(self, condition: Boolean, future: Future):
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

    def has(self, resid:  TYPE_NAMED_HASH) -> bool:
        pass

    def get_item_uv(self, resid:  TYPE_NAMED_HASH) -> TYPE_RECT:
        pass

    def get_original_size(self, resid:  TYPE_NAMED_HASH) -> tuple[float, float]:
        pass

    def get_pivot(self, resid:  TYPE_NAMED_HASH) -> tuple[float, float]:
        pass

    def add_importer(self, importer: AtlasImporter, readable: Optional[Readable] = None):
        pass

    def make_bitmap_bundle(self):
        pass


class BroadPhrase:
    pass


class AudioPlayer:
    PLAY_OPTION_DEFAULT = -1
    PLAY_OPTION_LOOP_OFF = 0
    PLAY_OPTION_LOOP_ON = 1

    def play(self, source: Union[str, Readable], options: int = PLAY_OPTION_DEFAULT):
        pass


class Font:

    FONT_STYLE_REGULAR = 0
    FONT_STYLE_BOLD = 1
    FONT_STYLE_ITALIC = 2
    FONT_STYLE_MONOCHROME = 4

    def __init__(self, size: str, style: int = 0):
        pass

    @property
    def style(self) -> int:
        return 0


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


class Observer(Runnable):
    def __init__(self):
        pass

    def update(self):
        pass

    def add_callback(self, callback: Callable, oneshot: bool = False, trigger_after: int = 1):
        pass


class Shader:
    pass


class Renderer:

    PRIORITY_UI = -10,
    PRIORITY_DEFAULT = 0,
    PRIORITY_UI_BLEND = 10,
    PRIORITY_UI_TEXT = 20
    PRIORITY_RENDER_LAYER = 30
    PRIORITY_CONTROL = 50

    def __init__(self, delegate: Optional['Renderer'] | list['Renderer'] = None):
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

    def make_disposable(self, disposed: Union[bool, "Boolean"] = None) -> 'Renderer':
        pass

    def make_visible(self, visibility: Union[bool, "Boolean", 'Visibility']) -> 'Renderer':
        pass

    def make_auto_release(self, ref_count: int = 1) -> 'Renderer':
        pass


class RenderPass(Renderer):
    def __init__(self, shader, vertex_buffer: Buffer, index_buffer: Buffer, draw_count: "Integer", render_mode: TYPE_ENUM, draw_procedure: TYPE_ENUM,
                 divided_uploaders: tuple[tuple[int, "Uploader"]] = tuple(), indirect_buffer: Optional[Buffer] = None):
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
    def position(self) -> Optional['Vec3']:
        return None

    @property
    def target(self) -> Optional['Vec3']:
        return None

    @property
    def up(self) -> Optional['Vec3']:
        return None

    @property
    def view(self) -> "Mat4":
        pass

    @view.setter
    def view(self, view: "Mat4"):
        pass

    @property
    def projection(self) -> "Mat4":
        pass

    @projection.setter
    def projection(self, projection: "Mat4"):
        pass

    @property
    def vp(self) -> "Mat4":
        return None

    def ortho(self, left_top: TYPE_VEC2, right_bottom: TYPE_VEC2, clip: TYPE_VEC2):
        pass

    def frustum(self, left, right, top, bottom, near, far):
        pass

    def perspective(self, fov, aspect, near, far):
        pass

    def look_at(self, position: TYPE_VEC3, target: TYPE_VEC3, up: TYPE_VEC3):
        pass

    def to_world_position(self, screen_position: TYPE_VEC3) -> TYPE_VEC3:
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

    def if_else(self, condition: "Boolean", negative: Union[str, 'String']) -> 'String':
        pass

    @staticmethod
    def format(msg: str, **kwargs) -> 'String':
        pass


class _Scalar(_Var):

    def at_least(self, least) -> Self:
        pass

    def at_most(self, most) -> Self:
        pass

    def fence(self, fence) -> Self:
        pass

    def dirty(self) -> Boolean:
        pass


class Numeric(_Scalar):
    def __init__(self, val):
        pass

    def approach(self, expectation) -> Self:
        pass

    def integral(self, t: Optional['Numeric'] = None) -> 'Numeric':
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


class Integer(_Scalar):
    REPEAT_NONE = 0
    REPEAT_REVERSE = 1
    REPEAT_LOOP = 4
    REPEAT_LAST = 8
    REPEAT_NOTIFY = 16

    def __init__(self, value: Union[int, "Integer", Numeric, list["Integer"]]):
        super().__init__(value)

    @staticmethod
    def repeat(array: list[int], repeat: int) -> "Integer":
        pass

    @staticmethod
    def to_repeat(repeat: str) -> int:
        pass

    def animate(self, interval: Union[Numeric, float] = None, duration: Union[Numeric, float] = None) -> "Integer":
        pass

    def __len__(self):
        pass

    def __getitem__(self, index: TYPE_INTEGER) -> "Integer":
        pass

    def __floordiv__(self, other: TYPE_INTEGER) -> "Integer":
        pass


class Discarded(Boolean):
    def __init__(self, discarded: TYPE_BOOLEAN = False):
        Boolean.__init__(self, discarded)

    def discard(self):
        pass

    def observe(self, observer: Observer) -> Boolean:
        pass

    def __bool__(self):
        return False


class Vec2(_Var):
    def __init__(self, x: TYPE_NUMERIC, y: Optional[TYPE_NUMERIC] = None):
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

    def integral(self, t: Optional[Numeric] = None) -> Self:
        pass

    def wrap(self) -> Self:
        return self

    def synchronize(self, discarded: Optional[Boolean] = None) -> Self:
        return self

    def attract(self, s0: Union[tuple, 'Vec2'], duration: float, t: Optional[Numeric] = None) -> Self:
        return self

    def fence(self, plane: Union['Vec3', tuple], observer: Union[Observer, Callable[[], None]]) -> Self:
        return self

    def atan2(self) -> Numeric:
        pass

    def normalize(self):
        pass

    def distance(self, other) -> Numeric:
        pass

    def distance2(self, other) -> Numeric:
        pass

    def hypot(self) -> Numeric:
        pass

    def to_size(self) -> 'Size':
        pass

    def lerp(self, other, t: TYPE_NUMERIC) -> Self:
        pass

    def sod(self, d0, k: float, z: float, r: float, t: Optional['Numeric'] = None):
        pass

    def extend(self, v):
        pass

    def __iter__(self):
        pass

    def __len__(self):
        return 2

    def __getitem__(self, i):
        pass

    def __str__(self):
        return '(%.1f, %.1f)' % (self._x, self._y)


class Vec3(Vec2):
    def __init__(self, x: TYPE_NUMERIC, y: Optional[TYPE_NUMERIC] = None, z: Optional[TYPE_NUMERIC] = None):
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
    def __init__(self, x: TYPE_NUMERIC, y: Optional[TYPE_NUMERIC] = None, z: Optional[TYPE_NUMERIC] = None, w: Optional[TYPE_NUMERIC] = None):
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


class Translation:
    def __init__(self, translate: TYPE_VEC3):
        pass

    def reset(self, position: TYPE_VEC3):
        pass

    def to_matrix(self) -> Mat4:
        pass


class Uploader:
    def __init__(self, delegate: Union[Numeric, Vec2, Vec3, Vec4, list[_Mat], list[tuple], list[int], list['Uploader'], Any], size: int = 0):
        pass

    @property
    def size(self):
        return 0

    def reserve(self, size: int) -> Self:
        pass

    def remap(self, size: int, offset: int = 0) -> Self:
        pass

    def repeat(self, length: int, stride: int = 0) -> Self:
        pass

    def put(self, offset: int, input_: 'Uploader'):
        pass

    def remove(self, offset: int):
        pass

    def wrap(self) -> 'Uploader':
        pass

    def reset(self, delegate: 'Uploader'):
        pass

    def mark_dirty(self):
        pass

    def dye(self, message: str) -> Self:
        return self


class AnimationInput(Uploader):

    def get_node_matrix(self, node_name: str) -> Mat4:
        pass

    def get_transform_variance(self, c: tuple[float, float, float], nodes: list[str]) -> list[float]:
        pass


class Animation:
    @property
    def ticks(self) -> int:
        return 0

    @property
    def tps(self) -> float:
        return 0

    @property
    def duration(self) -> float:
        return 0

    def get_local_transforms(self, t: TYPE_INTEGER | TYPE_NUMERIC) -> list[tuple[str, Mat4]]:
        pass

    def get_global_transform(self, node: "Node", t: TYPE_INTEGER | TYPE_NUMERIC) -> Mat4:
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

    @property
    def emission(self) -> Optional[MaterialTexture]:
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
    def parent_node(self) -> Optional["Node"]:
        pass

    @property
    def child_nodes(self) -> list['Node']:
        return []

    @property
    def meshes(self) -> list[Mesh]:
        return []

    @property
    def local_matrix(self) -> TYPE_M4:
        return ((0,) * 4, ) * 4

    @property
    def translation(self) -> TYPE_FLOAT3:
        pass

    @property
    def rotation(self) -> TYPE_FLOAT4:
        pass

    @property
    def scale(self) -> TYPE_FLOAT3:
        pass

    def find_child_node(self, name: str) -> Optional['Node']:
        pass


class Boundaries:

    @property
    def aabb_min(self) -> Vec3:
        return 0, 0, 0

    @property
    def aabb_max(self) -> Vec3:
        return 0, 0, 0

    @property
    def size(self) -> Vec3:
        return 0, 0, 0

    @property
    def center(self) -> Vec3:
        return 0, 0, 0


class Vertices:

    @property
    def length(self) -> int:
        return 0

    def make_uploader(self, shader: Shader, bounds: TYPE_VEC3) -> Uploader:
        pass


class Model:

    @property
    def content(self) -> Boundaries:
        return Boundaries()

    @property
    def occupy(self) -> Boundaries:
        return Boundaries()

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
    def root_node(self) -> Optional[Node]:
        return None

    @property
    def indices(self) -> Uploader:
        return Uploader(None)

    @property
    def vertices(self) -> Vertices:
        pass

    def find_node(self, name: str) -> Optional[Node]:
        pass

    def get_animation(self, name: str) -> Animation:
        pass


class MaterialBundle:
    def __init__(self, materials: list[Material] = None):
        pass

    @property
    def materials(self) -> list[Material]:
        pass

    def add_material(self, name: str, material: Material):
        pass


class ModelBundle:

    @property
    def material_bundle(self) -> MaterialBundle:
        pass

    @property
    def index_length(self) -> int:
        return 0

    @property
    def vertex_length(self) -> int:
        return 0

    def get_model(self, t: TYPE_NAMED_HASH) -> Model:
        pass

    def import_materials(self, t: TYPE_NAMED_HASH, manifest: str | Manifest):
        pass

    def import_model(self, t: TYPE_NAMED_HASH, manifest: str | Manifest, future: Optional[Future] = None):
        pass


class LevelLibrary:
    @property
    def id(self) -> int:
        return 0

    @property
    def name(self) -> str:
        return ''

    @property
    def size(self) -> Vec3:
        pass

    @property
    def shape(self) -> "Shape":
        pass

    @shape.setter
    def shape(self, shape: Shape):
        pass


class LevelObject:

    TYPE_INSTANCE = 0
    TYPE_MESH = 1
    TYPE_CAMERA = 2
    TYPE_LIGHT = 3

    @property
    def name(self) -> str:
        pass

    @property
    def type(self) -> int:
        pass

    @property
    def visible(self) -> bool:
        pass

    @property
    def position(self) -> TYPE_FLOAT3:
        pass

    @property
    def scale(self) -> Optional[TYPE_FLOAT3]:
        pass
    @property
    def shape(self) -> Optional[TYPE_FLOAT3]:
        pass
    @property
    def rotation(self) -> Optional[TYPE_FLOAT4]:
        pass

    @property
    def library(self) -> Optional[LevelLibrary]:
        pass

    @property
    def render_object(self) -> "RenderObject":
        pass

    @property
    def rigidbody(self) -> "Rigidbody":
        pass

    def create_render_object(self) -> "RenderObject":
        pass

    def create_rigidbody(self, collider: "Collider", body_type: int, shapes: dict[str, "Shape"], collision_filter: "CollisionFilter") -> "Rigidbody":
        pass


class LevelLayer:

    @property
    def name(self) -> str:
        pass

    @property
    def objects(self) -> list[LevelObject]:
        pass

    def get_object(self, name: str) -> Optional[LevelObject]:
        pass

    def create_render_objects(self, layer: Optional["Layer"] = None):
        pass

    def create_rigidbodies(self, collider: "Collider", body_type: int, collision_filter: Optional["CollisionFilter"] = None):
        pass


class Level:
    def __init__(self, src: str):
        pass

    @property
    def libraries(self) -> dict[int, LevelLibrary]:
        pass

    @property
    def layers(self) -> list[LevelLayer]:
        pass

    @property
    def cameras(self) -> dict[str, "Camera"]:
        pass

    def get_layer(self, name: str) -> LevelLayer:
        pass

    def get_camera(self, name: str) -> Optional[Camera]:
        pass


class RenderObject:
    def __init__(self, type: TYPE_NAMED_HASH, position: Optional[TYPE_VEC3] = None, size: Optional[TYPE_VEC3] = None, transform=None, varyings: Optional['Varyings'] = None, visible: Optional['Visibility'] = None, discarded: Optional[Boolean] = None):
        self._position = position
        self._size = size
        self._transform = transform
        self._discarded = discarded

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
    def discarded(self) -> Boolean:
        return self._discarded

    @discarded.setter
    def discarded(self, discarded: Boolean):
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

    def discard(self):
        pass

    def show(self):
        pass

    def hide(self):
        pass


class LayerContext:
    pass


class RenderLayer(Renderer):

    @property
    def context(self) -> LayerContext:
        return LayerContext()

    def make_layer(self, model_loader: Optional[ModelLoader] = None, position: Optional[Vec3] = None, visible: Optional[Boolean] = None,
                   discarded: Optional[Boolean] = None) -> "Layer":
        pass


class Layer:
    def __init__(self):
        pass

    @property
    def context(self) -> LayerContext:
        return LayerContext()

    def add_render_object(self, render_object: RenderObject, discarded: Boolean = None):
        pass

    def clear(self):
        pass


class Arena:

    @property
    def layers(self):
        pass

    @property
    def render_layers(self):
        pass


class Activity:

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

    def add_render_layer(self, render_layer: Renderer, discarded: Optional[Boolean] = None):
        pass

    def add_event_listener(self, event_listener: Union[Callable[["Event"], bool], 'EventListener'], priority: int = 0):
        pass

    def make_arena(self) -> Arena:
        pass


class Clock:

    @property
    def tick(self) -> int:
        return 0

    def duration(self) -> 'Numeric':
        pass

    def timeout(self, seconds: float) -> "Boolean":
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

    BUTTON_NONE = 0
    BUTTON_MOUSE_LEFT = 1
    BUTTON_MOUSE_MIDDLE = 2
    BUTTON_MOUSE_RIGHT = 3
    BUTTON_MOTION_POINTER1 = 4
    BUTTON_MOTION_POINTER2 = 5
    BUTTON_MOTION_POINTER3 = 6
    BUTTON_MOTION_POINTER4 = 7

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
    TYPE_NONE = 0
    TYPE_LINEAR_2D = 1
    TYPE_LINEAR_3D = 2

    def __init__(self, matrix: Mat4):
        pass

    @property
    def rotation(self) -> Vec4:
        pass

    @rotation.setter
    def rotation(self, rotation: Vec4):
        pass

    @property
    def scale(self) -> Vec3:
        pass

    @scale.setter
    def scale(self, v):
        pass

    @property
    def translation(self) -> Vec3:
        pass

    @translation.setter
    def translation(self, v):
        pass


class Transform2D(Transform):
    def __init__(self, rotation: Optional[TYPE_NUMERIC] = None, scale: Optional[TYPE_VEC2] = None, translation: Optional[TYPE_VEC2] = None):
        pass


class Transform3D(Transform):
    def __init__(self, rotation: Optional[TYPE_VEC4] = None, scale: Optional[TYPE_VEC3] = None, translation: Optional[TYPE_VEC3] = None):
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
    def hypot2(x, y):
        return 0

    @staticmethod
    def dot(x, y) -> Numeric:
        pass

    @staticmethod
    def distance(a, b):
        pass

    @staticmethod
    def rand() -> int:
        return 0

    @staticmethod
    def randf() -> float:
        return 0

    @staticmethod
    def randfv(a: Optional[Numeric], b: Optional[Numeric], is_volatile: bool = True) -> Numeric:
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
    def __init__(self, delegate: Optional["EventListener"]):
        pass

    def on_event(self, event: Event):
        pass

    def reset(self, listener: Any = None):
        pass

    def add_event_listener(self, listener: Any, disposed: Optional[Boolean] = None):
        pass

    def push_event_listener(self, listener: Any, disposed: Optional[Boolean] = None):
        pass


class EventDispatcher(EventListener):
    def __init__(self):
        super().__init__(None)

    def on_event(self, event):
        pass

    def on_key_event(self, code, on_press: Optional[TYPE_RUNNABLE] = None, on_release: Optional[TYPE_RUNNABLE] = None, on_repeat: Optional[TYPE_RUNNABLE] = None):
        pass

    def on_motion_event(self, on_press: Optional[EventListener] = None, on_release: Optional[EventListener] = None, on_click: Optional[EventListener] = None, on_move: Optional[EventListener] = None):
        pass


class Glyph:
    def __init__(self, _type: Union[Integer, int], position: Optional[Vec3] = None, transform: Optional[Transform] = None, varyings: Optional['Varyings'] = None, visible: Optional['Visibility'] = None, discarded: Optional[Discarded] = None):
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
    def __init__(self, render_layer: RenderLayer, text: String | str | None = None, position: Optional[TYPE_VEC3] = None, layout_param: Optional["LayoutParam"] = None, glyph_maker: Any = None, transform: Optional[Mat4] = None,
                 letter_spacing: float = 0, line_height: float = 0, line_indent: float = 0):
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

    def show(self, discarded: Optional[TYPE_BOOLEAN] = None):
        pass

    def set_rich_text(self, text: str, **kwargs):
        pass


class StringBundle:

    def get_string(self, resid: str) -> str:
        return ''

    def get_string_array(self, resid: str) -> List[str]:
        return []


class Color(Vec4):
    def __init__(self, r: float, g: float, b: float, a: float = 1.0):
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

    TYPE_NONE = 0
    TYPE_AABB = -1
    TYPE_BALL = -2
    TYPE_BOX = -3
    TYPE_CAPSULE = -4

    def __init__(self, id: TYPE_NAMED_HASH, size: Optional[TYPE_VEC3] = None, origin: Optional[TYPE_VEC3] = None):
        pass

    @property
    def type(self) -> NamedHash:
        pass

    @property
    def size(self) -> Optional[Vec3]:
        return None

    @property
    def origin(self) -> Optional[Vec3]:
        return None


class Rigidbody:
    BODY_TYPE_NONE = 0
    BODY_TYPE_KINEMATIC = 1
    BODY_TYPE_DYNAMIC = 2
    BODY_TYPE_STATIC = 3
    BODY_TYPE_RIGID = 3
    BODY_TYPE_SENSOR = 1 << 2
    BODY_TYPE_GHOST = 2 << 2

    @property
    def id(self) -> int:
        return 0

    @property
    def type(self) -> int:
        return 0

    @property
    def shape(self) -> Shape:
        pass

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
    def rotation(self) -> Optional[Vec4]:
        pass

    @property
    def width(self) -> float:
        return 0

    @property
    def height(self) -> float:
        return 0

    @property
    def discarded(self) -> Optional[Boolean]:
        pass

    @property
    def collision_callback(self):
        return None

    @collision_callback.setter
    def collision_callback(self, collision_callback: "CollisionCallback"):
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

    def discard(self):
        pass


class SurfaceController:

    def add_renderer(self, renderer: Renderer, discarded: Optional[Boolean] = None, visible: Optional[Boolean] = None, priority: int = Renderer.PRIORITY_DEFAULT):
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
    def rigid_body(self) -> Optional[Rigidbody]:
        return None


class NarrowPhrase:
    pass


class Collider:
    def create_body(self, body_type: int | Integer, shape: Optional[Shape], position, rotate=None, collision_filter: Optional[CollisionFilter] = None, discarded: Optional[Boolean] = None) -> Rigidbody:
        pass

    def create_shape(self, shape_id: TYPE_NAMED_HASH, size: Optional[TYPE_VEC3] = None, origin: Optional[TYPE_VEC3] = None) -> Shape:
        pass

    def ray_cast(self, ray_from, ray_to, collision_filter: Optional[CollisionFilter] = None) -> List[RayCastManifold]:
        pass


class StateActionStrand:
    def __init__(self, start: "State", end: "State"):
        pass


class StateAction:

    def __init__(self, state_machine: "StateMachine", strand: StateActionStrand, on_activate: Optional[Runnable] = None, on_deactivate: Optional[Runnable] = None):
        pass

    @property
    def strand(self) -> StateActionStrand:
        pass

    @property
    def start(self) -> "State":
        pass

    @property
    def end(self) -> "State":
        pass

    @property
    def on_activate(self) -> Optional[Runnable]:
        pass

    @on_activate.setter
    def on_activate(self, on_activate: Optional[Runnable]):
        pass

    @property
    def on_deactivate(self) -> Optional[Runnable]:
        pass

    @on_deactivate.setter
    def on_deactivate(self, on_deactivate: Optional[Runnable]):
        pass

    @property
    def activate(self) -> Runnable:
        pass

    @property
    def deactivate(self) -> Runnable:
        pass


class State:

    def __init__(self, on_activate: Optional[Runnable] = None, on_deactivate: Optional[Runnable] = None):
        pass

    @property
    def active(self) -> bool:
        return False


class StateMachine:
    def __init__(self, entry: State):
        pass

    def reset(self, state: State):
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


class LayoutLength:

    LENGTH_TYPE_AUTO = 0
    LENGTH_TYPE_PIXEL = 1
    LENGTH_TYPE_PERCENTAGE = 2

    def __init__(self, value: TYPE_NUMERIC | str, type: int = LENGTH_TYPE_AUTO):
        pass

    def value(self) -> TYPE_NUMERIC:
        pass

    def type(self) -> int:
        pass


class LayoutParam:
    DISPLAY_BLOCK = 0
    DISPLAY_FLOAT = 1
    DISPLAY_ABSOLUTE = 2

    JUSTIFY_CONTENT_FLEX_START = 0
    JUSTIFY_CONTENT_FLEX_END = 1
    JUSTIFY_CONTENT_CENTER = 2
    JUSTIFY_CONTENT_SPACE_BETWEEN = 3
    JUSTIFY_CONTENT_SPACE_AROUND = 4
    JUSTIFY_CONTENT_SPACE_EVENLY = 5

    ALIGN_AUTO = 0
    ALIGN_FLEX_START = 1
    ALIGN_CENTER = 2
    ALIGN_FLEX_END = 3
    ALIGN_STRETCH = 4
    ALIGN_BASELINE = 5
    ALIGN_SPACE_BETWEEN = 6
    ALIGN_SPACE_AROUND = 7

    FLEX_DIRECTION_COLUMN = 0
    FLEX_DIRECTION_COLUMN_REVERSE = 1
    FLEX_DIRECTION_ROW = 2
    FLEX_DIRECTION_ROW_REVERSE = 3

    FLEX_WRAP_NOWRAP = 0
    FLEX_WRAP_WRAP = 1
    FLEX_WRAP_WRAP_REVERSE = 2


class View:

    @property
    def discarded(self) -> Boolean:
        return Boolean(False)

    @property
    def layout_param(self) -> LayoutParam:
        return LayoutParam()

    def add_view(self, view: "View"):
        pass

    def find_view(self, name: str) -> Optional["View"]:
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
    def ref(self) -> Ref:
        pass

    @property
    def discarded(self) -> Discarded:
        pass

    def add_component(self, component: Any):
        pass

    def get_component(self, ctype: type):
        pass

    def discard(self):
        pass


class WithId:
    def __init__(self):
        pass

    @property
    def id(self) -> int:
        return 0


class WithTag:
    def __init__(self, tag: Any):
        pass

    @property
    def tag(self) -> Any:
        return None

    @tag.setter
    def tag(self, tag: Any):
        pass


class CollisionCallback:
    pass



class PathFinder:
    def __init__(self, behavior: Behavior, name: str = 'visit_adjacent_nodes'):
        pass

    def find_path(self, start: tuple[float, float, float], end: tuple[float, float, float]) -> list[tuple[float, float, float]]:
        pass


def __trace__():
    pass
