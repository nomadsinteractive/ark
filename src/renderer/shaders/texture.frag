
uniform sampler2D u_Texture0;

vec4 ark_main(vec2 texCoordinate)
{
    vec4 c = texture2D(u_Texture0, texCoordinate);
    if(c.a == 0)
        discard;
    return c;
}
