
#include <shaders/defines.h>

uniform mat4 u_VP;

vec4 ark_main(in vec2 pos) {
    return u_VP * vec4(pos.xy, 0.0, 1.0);
}
