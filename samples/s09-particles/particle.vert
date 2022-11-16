
uniform mat4 u_VP;

vec4 ark_main(in vec2 position, in vec2 velocity) {
    gl_PointSize = 8.0;
    return u_VP * vec4(position, 0, 1);
}
