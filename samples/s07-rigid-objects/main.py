import math
import random

from ark import dear_imgui, ApplicationFacade, Arena, Event, Integer, Collider, RenderObject, Size, Camera, Vec3, Numeric


class Application:
    def __init__(self, application: ApplicationFacade):
        self._down_x = 0
        self._down_y = 0
        self._application = application
        self._light_position = Vec3(100, 500, 0)
        self._resource_loader = self._application.create_resource_loader('main.xml')
        self._arena = self._resource_loader.load(Arena, 'main', c=self._application.camera, lp=self._light_position)
        self._application.arena = self._arena
        self._arena.add_event_listener(self.on_event)
        self._imgui = self._arena.resource_loader.refs.imgui
        self._world_box2d = self._resource_loader.load(Collider, 'b2World')
        self._world_bullet = self._resource_loader.load(Collider, 'btWorld')
        self._l001 = self._resource_loader.layers.l001
        self._l003 = self._resource_loader.layers.l003
        self._shape_id = Integer(0)
        self._collider_id = Integer(0)
        self._body_size = Numeric(50)
        self._body_ttl = Numeric(5)
        self._rigid_body_ground_b2 = self._world_box2d.create_body(Collider.BODY_TYPE_STATIC, Collider.BODY_SHAPE_BOX, (4.8, 0), Size(6.0, 1.0))
        self._rigid_body_ground_bt = self._world_bullet.create_body(Collider.BODY_TYPE_STATIC, Collider.BODY_SHAPE_BOX, (480, 0, 0), Size(600, 100, 600))
        self._shapes = [Collider.BODY_SHAPE_BALL, Collider.BODY_SHAPE_BOX, 3]

    @property
    def imgui(self):
        return self._imgui

    def on_event(self, event):
        action = event.action

        if action == Event.ACTION_DOWN:
            (self._down_x, self._down_y) = event.xy
        elif action == Event.ACTION_UP:
            if abs(event.x - self._down_x) + abs(event.y - self._down_y) < 10:
                self.on_click(event)
                return True
        return False

    def on_click(self, event: Event):
        shape_id = self._shape_id.val
        collider_id = self._collider_id.val
        render_object = [self.make_object_box2d, self.make_object_bullet][collider_id](shape_id, event)
        self._defer_dispose(render_object)

    def make_object_box2d(self, shape_id: int, event: Event) -> RenderObject:
        xy = (event.x / 100, event.y / 100)
        s = self._body_size / 100
        shape = self._shapes[shape_id]
        rigid_body = self._world_box2d.create_body(Collider.BODY_TYPE_DYNAMIC, shape, xy, Size(s, s))
        render_object = RenderObject(random.randrange(1, 100), None, Size(self._body_size, self._body_size), None)
        rigid_body.bind(render_object)
        self._l003.add_render_object(render_object)
        return render_object

    def make_object_bullet(self, shape_id: int, event: Event) -> RenderObject:
        xy = event.xy
        shape = self._shapes[shape_id]
        s = self._body_size.val
        s1 = s / [2, 100, 50][shape_id]
        rigid_body = self._world_bullet.create_body(Collider.BODY_TYPE_DYNAMIC, shape, xy, Size(s, s, s))
        render_object = RenderObject(self._shape_id.val + 1, None, Size(s1, s1, s1))
        rigid_body.bind(render_object)
        self._l001.add_render_object(render_object)
        return render_object

    def create_toolbox(self):
        builder = dear_imgui.RendererBuilder(self._imgui)
        builder.begin('RigidBodies')
        builder.text('Which collider engine shall we use?')
        builder.radio_button('Box2D', self._collider_id, 0)
        builder.same_line()
        builder.radio_button('Bullet3', self._collider_id, 1)
        builder.separator()
        builder.text('Click somewhere to create a RigidBody typed below:')
        builder.radio_button('Ball', self._shape_id, 0)
        builder.same_line()
        builder.radio_button('Box', self._shape_id, 1)
        builder.same_line()
        builder.radio_button('Duck', self._shape_id, 2)
        builder.slider_float('RigidBody size', self._body_size, 10, 100, '%.1f')
        builder.slider_float('RigidBody TTL', self._body_ttl, 5, 50, '%.1f')
        builder.slider_float3('Light Position', self._light_position, 0, 1000, '%.1f')
        builder.end()
        self._imgui.add_renderer(builder.build())

    @staticmethod
    def _make_camera() -> Camera:
        e = 500
        camera = Camera()
        camera.perspective(math.radians(45), 16 / 9, 0.1, 2000)
        camera.look_at(Vec3(0, 0, e), Vec3(0, 0, e - 100), Vec3(0, 1, 0))
        return camera

    def _defer_dispose(self, render_object: RenderObject):
        self._application.post(lambda: render_object.dispose(), self._body_ttl.val)


def main(app: Application):
    app.create_toolbox()


if __name__ == '__main__':
    main(Application(_application))
