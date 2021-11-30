
uniform mat4 u_VP;

vec4 ark_main(in vec4 position, in vec3 pre_trans, in vec3 scale, in vec4 quaternion, in vec3 post_trans) {
    return u_VP * model * position;
}
