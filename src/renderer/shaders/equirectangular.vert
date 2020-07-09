
uniform mat4 u_Projection;
uniform mat4 u_View;

vec4 ark_main(in vec4 position, out vec3 localPosition)
{
    localPosition = position.xyz;
    return u_Projection * u_View * position;
}
