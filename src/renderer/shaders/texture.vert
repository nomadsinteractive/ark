
uniform mat4 u_MVPMatrix;

vec4 ark_main(vec4 position, vec2 texCoordinate, out vec2 v_TexCoordinate) {
    v_TexCoordinate = texCoordinate;
    return u_MVPMatrix * position;
}
