"""

This module only servers as a mimic of the Ark runtime library.
Functionalities are limited or unimplemented.

Use it for:
1. Autocomplete
2. Documentation
3. Unit test(maybe)

"""


def logd(*args):
    print(*args)


def get_asset(filename):
    with open(filename, 'rt') as fp:
        return fp.read()


class _ArkObject:
    pass


class Object:
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

    def __int__(self):
        return 0

    def __float__(self):
        return 0.0


class Renderer:
    pass


class Camera:
    def __init__(self):
        pass

    def ortho(self, left, right, top, bottom, near, far):
        pass

    def perspective(self, fov, aspect, near, far):
        pass

    def look_at(self, position, target, up):
        pass


class Collider:

    BODY_SHAPE_AABB = -1
    BODY_SHAPE_BALL = -2
    BODY_SHAPE_BOX = -3

    BODY_TYPE_DYNAMIC = 0
    BODY_TYPE_KINEMATIC = 1
    BODY_TYPE_STATIC = 2

    BODY_FLAG_MANUAL_ROTATION = 4

    def create_body(self, type, shape, position, size=None, rotate=None):
        return None


class Tracker:
    def create(self, id, position, aabb):
        return position

    def remove(self, id):
        pass

    def search(self, position, aabb):
        return None


class Boolean(_Var):
    pass


class Integer(_Var):
    pass


class Numeric(_Var):
    def __init__(self, val):
        _Var.__init__(self, val)

    def expect(self, expectation):
        return None

    def make_stalker(self, s0, eta):
        return None

    def ternary(self, condition, _else):
        return None

    def __add__(self, other):
        return None

    def __mul__(self, other):
        return None

    def __sub__(self, other):
        return None

    def __truediv__(self, other):
        return None

    def __gt__(self, other):
        return False

    def __lt__(self, other):
        return False

    def __eq__(self, other):
        return False


class Expired:
    def __init__(self, v=False):
        pass


class Vec2:
    def __init__(self, x, y):
        self._x = x
        self._y = y

    @property
    def x(self):
        return self._x

    @x.setter
    def x(self, v):
        pass

    @property
    def y(self):
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
    def vx(self):
        return self._x

    @property
    def vy(self):
        return self._y

    def transform(self, transform, org):
        return self._x, self._y

    def fix(self):
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

    @property
    def xyz(self):
        return None

    def cross(self, other):
        return None

    def normalize(self):
        return None


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


class Mat3:
    def __init__(self, t=None, b=None, n=None):
        pass

    def __mul__(self, other):
        return None


class Layer:
    pass


class Arena:
    @staticmethod
    def load(clz, name, **kwargs):
        return _ArkObject()

    @property
    def references(self):
        return None


class Algorithm:
    @staticmethod
    def atan2(y, x):
        return 0


class Clock:
    pass


class Expectation:
    def __init__(self, expectation, onfire=None, fire_once=True):
        pass

    @property
    def val(self):
        return None

    @val.setter
    def val(self, v):
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
    CODE_KEYBOARD_RETURN = 44
    CODE_KEYBOARD_ESCAPE = 45
    CODE_KEYBOARD_BACKSPACE = 46
    CODE_KEYBOARD_TAB = 47
    CODE_KEYBOARD_SPACE = 48

    @property
    def action(self):
        return self.ACTION_KEY_DOWN

    @property
    def code(self):
        return self.CODE_NONE

    @property
    def x(self):
        return 0

    @property
    def y(self):
        return 0

    @property
    def timestamp(self):
        return 0


class Rotation:
    def __init__(self, value, direction=None):
        self._value = value
        self._direction = direction

    @property
    def rotation(self):
        return self._rotation

    @rotation.setter
    def rotation(self, v):
        self._rotation = v

    @property
    def direction(self):
        return self._direction


class Transform:
    def __init__(self, rotate=None, scale=None, translate=None):
        self._rotate = rotate
        self._scale = scale
        self._translate = translate

    @property
    def rotate(self):
        return self._rotate

    @rotate.setter
    def rotate(self, v):
        self._rotate = v

    @property
    def scale(self):
        return self._scale

    @scale.setter
    def scale(self, v):
        self._scale = v

    @property
    def translate(self):
        return self._translate

    @translate.setter
    def translate(self, v):
        self._translate = v


class RenderObject:
    def __init__(self, t, pos=None, size=None, transform=None, varyings=None):
        pass

    @property
    def type(self):
        return 0

    @type.setter
    def type(self, val):
        pass

    @property
    def position(self):
        return None

    @position.setter
    def position(self, v):
        pass

    @property
    def x(self):
        return 0

    @property
    def y(self):
        return 0

    @property
    def transform(self):
        return None

    @property
    def varyings(self):
        return None

    @varyings.setter
    def varyings(self, v):
        pass

    @property
    def xy(self):
        return 0, 0

    def absorb(self, o):
        pass

    def expire(self):
        pass


class Math:

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
    def randf():
        return 0

    @staticmethod
    def atan2(dy, dx):
        return 0

    @staticmethod
    def quadratic(a, b, c):
        return 0, 0

    @staticmethod
    def projectile(dx, dy, v, g, sid):
        return 0, 0


class Size:
    def __init__(self, w, h):
        pass

    @property
    def width(self):
        return 0

    @property
    def height(self):
        return 0


class TileMap:
    def __init__(self, layer, w, h, tw, th):
        pass

    def set_tile(self, row_id, col_id, obj):
        pass

    def get_tile_type(self, row, col):
        return 0

    @property
    def tile_width(self):
        return 0

    @property
    def tile_height(self):
        return 0

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


class Function:
    def __init__(self, d):
        pass


class EventDispatcher:
    def __init__(self):
        pass

    def on_key_event(self, code, on_press, on_release, on_click, on_repeat):
        pass

    def on_motion_event(self, on_down, on_up, on_click, on_move):
        pass

    def on_event(self, event):
        pass


class Characters:
    def __init__(self, layer, text_scale=1.0, letter_spacing=0, line_height=0, line_indent=0):
        pass


class StringBundle:
    def __init__(self):
        pass


class Varyings:
    def __init__(self, shader):
        pass

    def add(self, name, obj):
        pass


class RenderLayer:
    def __init__(self, layer):
        pass

    def add_render_object(self, obj):
        pass


class Expried:
    def __init__(self, e):
        pass

    def expired(self):
        return False

    def expire(self):
        pass


class Color:
    def __init__(self, v):
        pass

    def assign(self, other):
        pass

    @propertry
    def value(self):
        return None


class Platform:
    @staticmethod
    def get_user_storage_path(p):
        return None


def __trace__():
    pass
