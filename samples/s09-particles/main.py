from ark import dear_imgui, ApplicationFacade, Event, Arena, Math, Vec2, Integer


class Application:
    PARTICLE_COUNT_MAX = 512

    def __init__(self, application: ApplicationFacade):
        self._application = application
        self._resource_loader = self._application.create_resource_loader('main.xml')
        self._mouse_down = Integer(0)
        self._particle_count = Integer(self.PARTICLE_COUNT_MAX)
        self._arena = self._resource_loader.load(Arena, 'main', mp=application.cursor_position, pc=self._particle_count, dt=1 / 60, md=self._mouse_down,
                                                 pos=self.make_default_pos())
        self._application.arena = self._arena
        self._arena.add_event_listener(self.on_event)
        self._imgui = self._arena.resource_loader.refs.imgui

    def on_event(self, event):
        if event.action in (Event.ACTION_UP, Event.ACTION_DOWN):
            md = self._mouse_down.val
            if event.action == Event.ACTION_DOWN:
                self._mouse_down.set(md + 1)
                if md == 0:
                    self._application.application_controller.set_mouse_capture(True)
            else:
                if md == 1:
                    self._application.application_controller.set_mouse_capture(False)
                self._mouse_down.set(md - 1)
        return False

    def create_toolbox(self):
        builder = dear_imgui.RendererBuilder(self._imgui)
        builder.begin('Particles')
        builder.text('This demo uses the following to achieve a particle system:')
        builder.bullet_text('Compute Shader')
        builder.bullet_text('Shader Storage Buffer Object')
        builder.separator()
        builder.slider_int('Paricle Count', self._particle_count, 1, self.PARTICLE_COUNT_MAX, f'%d / {self.PARTICLE_COUNT_MAX}')
        builder.end()
        self._imgui.add_renderer(builder.make_renderer())

    @staticmethod
    def make_default_pos() -> Vec2:
        return Vec2(Math.randv() * 960, Math.randv() * 540)


def main(app: Application):
    app.create_toolbox()


if __name__ == '__main__':
    main(Application(_application))
