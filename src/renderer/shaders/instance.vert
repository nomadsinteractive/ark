
uniform mat4 u_VP;

vec4 ark_main(vec4 position, vec3 normal, vec3 tangent, mat4 model, out vec3 v_WorldPosition, out vec3 v_WorldNormal, out vec3 v_WorldTangent) {
    vec4 mp = model * position;
    v_WorldPosition = vec3(mp);
    v_WorldNormal = mat3(model) * normal;
    v_WorldTangent = mat3(model) * tangent;
    return u_VP * mp;
}
