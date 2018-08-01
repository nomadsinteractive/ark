
uniform mat4 u_VP;

vec4 ark_main(vec4 position, vec3 normal, mat4 model, out vec3 v_ModelNormal, out vec3 v_ModelPosition) {
    vec4 mp = model * position;
    v_ModelPosition = vec3(mp);
    v_ModelNormal = mat3(model) * normal;
    return u_VP * mp;
}
