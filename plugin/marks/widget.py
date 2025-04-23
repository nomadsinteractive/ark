import math

from ark import Rotation, Vec3, Numeric, Vec4, RotationEuler, RotationAxisTheta
from ark import dear_imgui


class RotationEditor:
    def __init__(self, rotation: Rotation):
        self._rotation = rotation
        self._axis = Vec3(1, 0, 0)
        self._theta = Numeric(math.pi / 2)
        self._euler_pitch = Numeric(0)
        self._euler_yaw = Numeric(0)
        self._euler_roll = Numeric(0)
        self._quaternion = Vec4(0, 0, 0, 1)
        self._quaternion_real = Numeric(0)
        self._quaternion_ijk = Vec3(1, 0, 0)

        self._rotation_axis = RotationAxisTheta(self._axis, self._theta)
        self._rotation_euler = RotationEuler(self._euler_pitch, self._euler_yaw, self._euler_roll)

    def build(self, builder: dear_imgui.WidgetBuilder):
        builder.begin_tab_bar('Rotation', 0)
        axis_activated = builder.begin_tab_item('Axis-Angle')
        builder.slider_float3('Axis', self._axis, -1, 1)
        builder.slider_float('Theta', self._theta, -math.tau, math.tau)
        builder.end_tab_item()
        euler_activated = builder.begin_tab_item("Euler")
        builder.slider_float('Pitch', self._euler_pitch, -math.tau, math.tau)
        builder.slider_float('Yaw', self._euler_yaw, -math.tau, math.tau)
        builder.slider_float('Roll', self._euler_roll, -math.tau, math.tau)
        builder.end_tab_item()
        quaternion_activated = builder.begin_tab_item("Quaternion")
        builder.slider_float4('Quaternion', self._quaternion, -1, 1)
        builder.end_tab_item()
        builder.end_tab_bar()
        self._rotation.reset(self._rotation_axis.if_else(axis_activated, self._rotation_euler.if_else(euler_activated, self._quaternion)))
