
uniform sampler2D u_Texture0;

vec4 ark_main(in vec2 imguiUV, in vec4b color)
{
    return texture2D(u_Texture0, imguiUV) * color;
}
