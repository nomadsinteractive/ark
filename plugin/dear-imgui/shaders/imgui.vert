
#include <shaders/types.h>

uniform mat4 u_MVP;

vec4 ark_main(in vec2 pos) {
    return u_MVP * vec4(pos.xy, 0.0, 1.0);
}
