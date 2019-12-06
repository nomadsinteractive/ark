
uniform mat4 u_VP;

vec4 ark_main(in vec4 position) {
    return u_VP * position;
}
