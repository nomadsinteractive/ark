
uniform mat4 u_MVP;

vec4 ark_transform(vec3 position, vec3 pivot, vec3 scale, vec3 translate) {
    return (position - pivot) * scale + translate;
}

vec4 ark_main(vec3 position, vec3 pivot, vec3 scale, vec3 translate) {
    return u_MVP * position;
}
