
uniform samplerCube u_Texture0;

vec4 ark_main(vec3 texDirection)
{
    return textureCube(u_Texture0, texDirection);
}
