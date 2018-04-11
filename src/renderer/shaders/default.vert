
uniform mat4 u_MVP;

vec4 ark_main(vec3 position) {
    return u_MVP * vec4(position, 1.0);
}
