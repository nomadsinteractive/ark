
def logd(*args):
    pass


def get_asset(filename):
    with open(filename, 'rt') as fp:
        return fp.read()


class _ArkObject:
    pass


class Object:
    pass


class Var:
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


class Collider:

    BODY_SHAPE_BOX = 0
    BODY_SHAPE_CIRCLE = 1

    BODY_TYPE_DYNAMIC = 0
    BODY_TYPE_KINEMATIC = 1
    BODY_TYPE_STATIC = 2

    def create_body(self, type, shape, position, size):
        return None


class Boolean(Var):
    pass


class Integer(Var):
    pass


class Numeric(Var):
    pass


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

    @vx.setter
    def vx(self, v):
        pass

    @property
    def vy(self):
        return self._y

    @vy.setter
    def vy(self, v):
        pass

    def transform(self, transform, org):
        return self._x, self._y

    def fix(self):
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
    def vz(self, v):
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


class Layer:
    pass


class Arena:
    @staticmethod
    def load(clz, name, **kwargs):
        return _ArkObject()


class Algorithm:
    @staticmethod
    def atan2(y, x):
        return 0


class Clock:
    pass


class Expectation:
    def __init__(self, expectation, onfire):
        pass

class Event:

    ACTION_KEY_DOWN = 0
    ACTION_KEY_UP = 1
    ACTION_KEY_REPEAT = 2
    ACTION_DOWN = 3
    ACTION_UP = 4
    ACTION_MOVE = 5
    ACTION_CANCEL = 6
    ACTION_BACK_PRESSED = 7

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


class Transform:
    def __init__(self, rotation=None, scale=None, translation=None):
        pass


class RenderObject:
    def __init__(self, t, pos=None, size=None, transform=None, filter=None):
        pass

    @property
    def type(self):
        return 0

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
    def filter(self):
        return None

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
    def __init__(self, layer, scroller, w, h, tw, th):
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
    def __init__(self):
        pass


class StringBundle:
    pass


class Filter:
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


class Range:
    pass


class Partition:
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
