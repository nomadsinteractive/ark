import ark

from ark import dear_imgui


def show_my_window(builder):

    def onclick():
        ark.logd('abc')

    is_happy = ark.Boolean(False)
    value = ark.Integer(0)
    numeric = ark.Numeric(0)
    vec2 = ark.Vec2(0, 0)
    vec3 = ark.Vec3(0, 0, 0)
    vec4 = ark.Vec4(0, 0, 0, 0)

    builder.begin('test')
    builder.bullet_text('Hello world')
    builder.indent()
    builder.bullet_text('Hello world 1')
    builder.bullet_text('Hello world 2')
    builder.unindent()
    builder.spacing()
    builder.spacing()
    builder.button('abc').set_callback(onclick)
    builder.same_line()
    builder.button('exit').set_callback(_application.exit)
    builder.same_line()
    builder.button('def')
    builder.same_line()
    builder.small_button('ghi')
    builder.separator()
    builder.input_float('input_float', numeric)
    builder.slider_float('slider_float', numeric, 0, 1)
    builder.slider_float2('vec2', vec2, 0, 1)
    builder.slider_float3('vec3', vec3, 0, 1)
    builder.color_edit3('color_edit3', vec3)
    builder.color_picker3('color_picker3', vec3)
    builder.slider_float4('vec4', vec4, 0, 1)
    builder.color_edit4('color_edit4', vec4)
    builder.color_picker4('color_picker4', vec4)
    builder.end()
    builder.begin('add')
    builder.checkbox('happy', is_happy)
    builder.radio_button('A0', value, 0)
    builder.same_line()
    builder.radio_button('A1', value, 1)
    builder.same_line()
    builder.radio_button('A2', value, 2)
    builder.combo('Combo1', value, ['A0', 'A1', 'A2'])
    builder.end()


def main():
    resource_loader = _application.create_resource_loader('main.xml')
    arena = resource_loader.load(ark.Arena, 'main', mouse=ark.Vec2(300, 300))
    _application.arena = arena
    imgui = arena.resource_loader.refs.imgui

    builder = dear_imgui.RendererBuilder()
    # builder.show_about_window()
    show_my_window(builder)
    builder.show_demo_window()

    imgui.add_renderer(builder.build())


if __name__ == '__main__':
    main()
