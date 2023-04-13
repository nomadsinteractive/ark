import random

import ark
from ark import RenderObject, Transform, Rotation, ApplicationFacade, Integer, Arena, dear_imgui, Vec2, Numeric, Observer


class Application:

    RENDER_OBJECT_MAX_COUNT = 256

    def __init__(self, application: ApplicationFacade):
        self._application = application
        self._resource_loader = self._application.create_resource_loader('main.xml')
        self._render_object_count = Integer(self.RENDER_OBJECT_MAX_COUNT // 2)
        self._max_create_per_cycle = Integer(self.RENDER_OBJECT_MAX_COUNT // 16)
        self._max_delete_per_cycle = Integer(self.RENDER_OBJECT_MAX_COUNT // 16)
        self._cycle_interval = Numeric(4.)
        self._arena = self._resource_loader.load(Arena, 'main')
        self._application.arena = self._arena
        self._imgui = self._arena.resource_loader.refs.imgui
        self._arena.add_event_listener(self._imgui)
        self._render_objects = []
        self._create_render_objects(int(self._render_object_count))
        application.expect(self._render_object_count != len(self._render_objects), Observer(self._on_render_object_number_changed))
        application.post(self._on_cycle, float(self._cycle_interval))

    def create_toolbox(self):
        builder = dear_imgui.WidgetBuilder(self._imgui)
        builder.begin('RenderObjects')
        builder.text('This demo demonstrates how to create and delete renderable objects')
        builder.separator()
        hint = f'%d / {self.RENDER_OBJECT_MAX_COUNT}'
        builder.slider_int('Number of RenderObjects', self._render_object_count, 1, self.RENDER_OBJECT_MAX_COUNT, hint)
        builder.slider_int('Max Create Per Cycle', self._max_create_per_cycle, 1, self.RENDER_OBJECT_MAX_COUNT, hint)
        builder.slider_int('Max Delete Per Cycle', self._max_delete_per_cycle, 1, self.RENDER_OBJECT_MAX_COUNT, hint)
        builder.slider_float('Cycle Interval', self._cycle_interval, 1, 10, '%.2f')
        builder.end()
        self._imgui.add_renderer(builder.make_widget().to_renderer())

    def _on_cycle(self):
        self._application.post(self._on_cycle, float(self._cycle_interval))
        delete_count = random.randint(0, int(self._max_delete_per_cycle))
        if delete_count:
            self._delete_render_objects(delete_count)
        create_count = random.randint(0, int(self._max_create_per_cycle))
        if create_count:
            self._create_render_objects(create_count)

    def _create_render_objects(self, create_count: int):
        l03 = self._resource_loader.layers.l03
        t = self._application.clock.duration()
        for i in range(create_count):
            render_object = RenderObject(random.randrange(0, 128))
            render_object.position = self.make_default_pos()
            render_object.transform = Transform(Transform.TYPE_LINEAR_2D, Rotation(t * random.uniform(.1, 2)))
            l03.add_render_object(render_object)
            self._render_objects.append(render_object)
        self._render_object_count.set(len(self._render_objects))

    def _delete_render_objects(self, delete_count: int):
        delete_samples = set(random.sample(self._render_objects, min(len(self._render_objects), delete_count)))
        self._render_objects = [i for i in self._render_objects if i not in delete_samples]
        self._render_object_count.set(len(self._render_objects))
        for i in delete_samples:
            i.dispose()

    def _on_render_object_number_changed(self):
        render_objects_changed_number = int(self._render_object_count - len(self._render_objects))
        if render_objects_changed_number > 0:
            self._create_render_objects(render_objects_changed_number)
        elif render_objects_changed_number < 0:
            self._delete_render_objects(-render_objects_changed_number)
        self._application.expect(self._render_object_count != len(self._render_objects), Observer(self._on_render_object_number_changed))

    @staticmethod
    def make_default_pos() -> Vec2:
        return Vec2(random.uniform(0, 960), random.uniform(0, 340))


def main(application: ApplicationFacade):
    app = Application(application)
    app.create_toolbox()


if __name__ == '__main__':
    main(_application)
