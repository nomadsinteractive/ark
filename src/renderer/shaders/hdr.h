vec4 hdr_to_ldr(vec3 color)
{
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));
    return vec4(color, 1.0);
}
