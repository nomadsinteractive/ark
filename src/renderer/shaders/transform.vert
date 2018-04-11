
uniform mat4 u_MVP;

vec4 ark_main(vec4 position, mat4 transform) {
    return u_MVP * transform * position;
}
