
uniform sampler2D u_Texture0;

vec4 ark_main(vec2 texCoordinate, vec4 color)
{
    float gamma = 2.2;
    vec4 texColor = texture2D(u_Texture0, texCoordinate);
    vec3 fragColor = pow(texColor.rgb * color.rgb, vec3(1.0 / gamma));
    return vec4(fragColor, texColor.a);
}
