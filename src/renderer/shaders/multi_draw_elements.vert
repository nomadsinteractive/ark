
uniform mat4 u_VP;

vec4 ark_main(in vec4 position, in mat4 model) {
    return u_VP * model * position;
}
