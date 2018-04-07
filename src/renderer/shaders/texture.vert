
uniform mat4 u_MVPMatrix;

vec4 ark_main(vec4 position) {
    return u_MVPMatrix * position;
}
