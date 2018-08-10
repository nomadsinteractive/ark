
uniform samplerCube u_Texture0;

vec4 ark_main(vec3 texDirection)
{
    vec3 color = textureCube(u_Texture0, texDirection).rgb;
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));
    return vec4(color, 1.0);
}
