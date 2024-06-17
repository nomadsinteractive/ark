
#include <shaders/defines.h>

uniform sampler2D u_Texture0;

vec4 ark_main(in vec2 uv, in vec4b color)
{
    return texture2D(u_Texture0, uv) * color;
}
