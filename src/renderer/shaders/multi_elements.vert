
uniform mat4 u_VP;
uniform mat4 u_ModelMatrix[32];

vec4 ark_main(in vec4 position, in int modelId) {
    return u_VP * u_ModelMatrix[modelId] * position;
}
