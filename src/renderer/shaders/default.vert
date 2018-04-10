
uniform mat4 u_MVP;

vec4 ark_main(vec4 position) {
    return u_MVP * position;
}
