
uniform mat4 u_Projection;
uniform mat4 u_View;

vec4 ark_main(in vec4 position, out vec3 texDirection) {
    texDirection = position.xyz;
    return (u_Projection * mat4(mat3(u_View)) * position).xyww;
}
