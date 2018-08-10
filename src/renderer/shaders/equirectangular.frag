
uniform sampler2D u_Texture0;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

vec4 ark_main(in vec3 localPosition)
{		
    vec2 uv = SampleSphericalMap(normalize(localPosition));
    vec3 color = texture(u_Texture0, vec2(uv.x, 1.0 - uv.y)).rgb;
    return vec4(color, 1.0);
}
