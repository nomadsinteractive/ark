
uniform mat4 u_VP;
uniform mat4 u_Model;

uniform vec3 u_LightPos;

uniform vec4 u_LightColor;
uniform vec4 u_AmbientLightColor;
uniform float u_LightFadingFactor;

vec4 ark_main(vec4 position, vec3 normal, out vec4 v_Color) {
    vec3 modelViewVertex = vec3(u_Model * position);
    vec3 modelViewNormal = vec3(u_Model * vec4(normal, 0.0));
    vec3 lightVector = normalize(u_LightPos - modelViewVertex);

    float distance = length(u_LightPos - modelViewVertex) / u_LightFadingFactor;
    float diffuse = max(dot(modelViewNormal, lightVector), 0.0) * (1.0 / (1.0 + (distance * distance)));

    v_Color = min(u_LightColor * diffuse + u_AmbientLightColor, 1.0);

    return u_VP * position;
}
