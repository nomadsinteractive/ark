
uniform mat4 u_VP;

vec4 ark_main(vec4 position, mat4 model) {
    return u_VP * model * position;
}
