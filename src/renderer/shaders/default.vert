
uniform mat4 u_MVP;

vec4 ark_main(in vec4 position) {
    return u_MVP * position;
}
