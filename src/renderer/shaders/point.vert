
uniform mat4 u_MVPMatrix;

vec4 ark_main(vec4 position, float pointSize, vec2 texCoordinate, out vec2 v_TexCoordinate) {
    gl_PointSize = pointSize;
    v_TexCoordinate = texCoordinate;
    return u_MVPMatrix * position;
}
