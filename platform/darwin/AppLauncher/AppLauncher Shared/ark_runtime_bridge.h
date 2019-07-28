#ifndef ARK_RUNTIME_BRIDGE_H_
#define ARK_RUNTIME_BRIDGE_H_

#include <stdint.h>

void ark_runtime_on_create();
void ark_runtime_on_surface_created(void* view);
void ark_runtime_on_surface_changed(int32_t surfaceWidth, int32_t surfaceHeight, int32_t viewWidth, int32_t viewHeight);
void ark_runtime_on_surface_draw();

void ark_runtime_on_resume();
void ark_runtime_on_pause();

void ark_runtime_on_touch(uint32_t type, float x, float y, uint32_t timestamp, uint32_t orgBottomLeft);

void ark_runtime_get_render_resolution(float* width, float* height);

#endif /* ShaderTypes_h */
