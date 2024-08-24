#ifndef _ARK_SHADERS_DEFINES_H_
#define _ARK_SHADERS_DEFINES_H_

#define vec3b   vec3
#define vec4b   vec4

#define divisor(x)

#define u_VP   ${camera.uVP}
#define u_View   ${camera.uView}
#define u_Projection   ${camera.uProjection}

#define texture2D(x, y)   texture(x, y)
#define textureCube(x, y) texture(x, y)

#endif
