
uniform mat4 u_VP;

vec4 ark_main(vec4 position, float pointSize) {
    gl_PointSize = pointSize;
    return u_VP * position;
}
