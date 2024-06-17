#include <shaders/defines.h>

uniform mat4 u_VP;

vec4 ark_main(in vec4 position, in divisor(1) mat4 model) {
    return u_VP * model * position;
}
