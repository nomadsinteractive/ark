
uniform mat4 u_VP;

vec4 ark_main(vec4 position, vec3 normal, vec3 tangent, in divisor(1) mat4 model, out vec3 worldPosition, out vec3 worldNormal, out vec3 worldTangent) {
    vec4 mp = model * position;
    worldPosition = vec3(mp);
    worldNormal = mat3(model) * normal;
    worldTangent = mat3(model) * tangent;
    return u_VP * mp;
}
