
uniform mat4 u_PMatrix;
uniform mat4 u_MVMatrices[32];

uniform vec3 u_LightPos;

uniform vec4 u_LightColor;
uniform vec4 u_AmbientLightColor;
uniform float u_LightFadingFactor;

attribute vec4 a_Position;
attribute vec3 a_Normal;
attribute vec2 a_TexCoordinate;
attribute float a_MatrixId;

varying vec4 v_Color;
varying vec2 v_TexCoordinate;

void main()
{
    int id = int(a_MatrixId);
    mat4 mv = u_MVMatrices[id];
    vec3 modelViewVertex = vec3(mv * a_Position);
    vec3 modelViewNormal = vec3(mv * vec4(a_Normal, 0.0));
    vec3 lightVector = normalize(u_LightPos - modelViewVertex);

    float distance = length(u_LightPos - modelViewVertex) / u_LightFadingFactor * 10.0;
    float diffuse = max(dot(modelViewNormal, lightVector), 0.0) * (1.0 / (1.0 + (0.2 * distance * distance)));

    v_Color = min(u_LightColor * diffuse + u_AmbientLightColor, 1.0);
    v_TexCoordinate = a_TexCoordinate;

    gl_Position = u_PMatrix * mv * a_Position;
}
