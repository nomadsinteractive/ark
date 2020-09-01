import math

import ark
from ark import ApplicationFacade, Vec3, Camera, Math, Numeric, Mat3, Event, Size


class AppWorld:
    def __init__(self, application: ApplicationFacade, resolution: Size, target, light_position, radius):
        self._applicaiton = application
        self._resolution = resolution
        self._target = target
        self._radius = radius
        self._position = Vec3(target.x, -radius, target.z)
        self._alpha = -math.pi / 2
        self._beta = 0
        self._camera = Camera()
        self._camera.perspective(math.pi / 4, resolution.width / resolution.height, 100, 1000)
        self._camera.look_at(self._position, self._target, Vec3(0, 0, 1))
        self._tangent_space = self._create_tangent_matrix()
        self._light_position_impl = light_position
        self._light_position = self._position + (self._tangent_space * light_position)

    @property
    def target(self):
        return self._target

    @property
    def position(self):
        return self._position

    @property
    def camera(self):
        return self._camera

    @property
    def radius(self):
        return self._radius

    @radius.setter
    def radius(self, val):
        self._radius = val
        self.update_position()

    @property
    def light_position(self):
        return self._light_position

    def set_alpha_speed(self, va):
        self._alpha = self._alpha + self._applicaiton.clock.duration() * va
        yradius = self._radius * Math.cos(self._beta)

        self._position.x = self._target.x + yradius * Math.cos(self._alpha)
        self._position.y = self._target.y + yradius * Math.sin(self._alpha)

    def set_beta_speed(self, vb):
        self._beta = (self._beta + self._applicaiton.clock.duration() * vb).boundary(math.pi / -2 + 0.001 if vb < 0 else math.pi / 2 - 0.001)
        self.update_position()

    def update_position(self):
        yradius = self._radius * Math.cos(self._beta)

        self._position.x = self._target.x + yradius * Math.cos(self._alpha)
        self._position.y = self._target.y + yradius * Math.sin(self._alpha)
        self._position.z = self._target.z + self._radius * Math.sin(self._beta)

    def fix(self):
        self._position.fix()
        self._alpha = float(self._alpha)
        self._beta = float(self._beta)

    def on_event(self, event: Event):
        if event.action == Event.ACTION_MOVE:
            self._light_position_impl.x = event.x - self._resolution.width / 2
            self._light_position_impl.y = event.y - self._resolution.height / 2
        elif event.action == Event.ACTION_KEY_DOWN:
            if event.code == Event.CODE_KEYBOARD_A:
                self.set_alpha_speed(-2.2)
            elif event.code == Event.CODE_KEYBOARD_D:
                self.set_alpha_speed(2.2)
            elif event.code == Event.CODE_KEYBOARD_W:
                self.set_beta_speed(2.2)
            elif event.code == Event.CODE_KEYBOARD_S:
                self.set_beta_speed(-2.2)
        elif event.action == Event.ACTION_DOWN:
            ark.logd('light_position = %s', self._light_position.xyz)
        elif event.action == Event.ACTION_KEY_UP:
            self.fix()
        elif event.action == Event.ACTION_WHEEL:
            self.radius -= (event.x * 40)

    def _create_tangent_matrix(self):
        n = (self._target - self._position).normalize()
        t = Vec3(-n.vz, Numeric(0), n.vx + 0.01).normalize()
        b = -n.cross(t).normalize()
        return Mat3(t, b, n)
