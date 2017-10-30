
uniform sampler2D u_Texture0;

vec4 ark_main(vec2 texCoordinate)
{
    return texture2D(u_Texture0, texCoordinate);
}
