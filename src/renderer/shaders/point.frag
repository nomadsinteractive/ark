
uniform sampler2D u_Texture0;

uniform float u_ParticleWidth;
uniform float u_ParticleHeight;

vec4 ark_main(vec2 texCoordinate)
{
    return texture2D(u_Texture0, texCoordinate + (gl_PointCoord - vec2(0.5)) * vec2(u_ParticleWidth, u_ParticleHeight));
}
