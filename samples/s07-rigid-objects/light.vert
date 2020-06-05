
uniform mat4 u_VP;

uniform vec3 u_LightPos;

uniform vec3 u_LightColor;
uniform vec3 u_AmbientLightColor;
uniform vec3 u_LightAttenuation;

vec4 ark_main(in vec4 position, in vec3 normal, in mat4 model, out vec4 v_Color) {
    vec4 worldPosition = model * position;
    vec3 modelViewVertex = vec3(worldPosition);
    vec3 modelViewNormal = vec3(model * vec4(normal, 0.0));
    vec3 lightVector = normalize(u_LightPos - modelViewVertex);

    float distance = length(u_LightPos - modelViewVertex);
    float fatt = u_LightAttenuation[0] + u_LightAttenuation[1] * distance + u_LightAttenuation[2] * distance * distance;
    float diffuse = max(dot(modelViewNormal, lightVector), 0.0) / fatt;

    v_Color = vec4(min(u_LightColor * diffuse + u_AmbientLightColor, 1.0), 1.0);
    return u_VP * worldPosition;
}
