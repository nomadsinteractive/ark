
uniform sampler2D u_Texture0;

vec4 ark_main(vec2 texCoordinate)
{
    vec4 c = texture2D(u_Texture0, texCoordinate);
    return vec4(1.0, 1.0, 1.0, c.r);
}
