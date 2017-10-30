
uniform mat4 u_MVPMatrix;

vec4 ark_main(vec4 position, float pointSize) {
    gl_PointSize = pointSize;
    return u_MVPMatrix * position;
}
