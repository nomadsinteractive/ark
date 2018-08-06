
vec4 ark_main(in vec4 position, out vec3 v_TexDirection) {
    v_TexDirection = position.xyz;
    return position.xyww;
}
