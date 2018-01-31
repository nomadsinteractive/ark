cmake_minimum_required (VERSION 2.8.11)
project (math-neon)

enable_language (C)

set(LOCAL_MATH_NEON_SRC_LIST
	math_acosf.c
	math_asinf.c
	math_atan2f.c
	math_atanf.c
	math_ceilf.c
	math_cosf.c
	math_coshf.c
	math_expf.c
	math_fabsf.c
	math_floorf.c
	math_fmodf.c
	math_invsqrtf.c
	math_ldexpf.c
	math_log10f.c
	math_logf.c
	math_mat2.c
	math_mat3.c
	math_mat4.c
	math_modf.c
	math_neon.h
	math_powf.c
	math_runfast.c
	math_sincosf.c
	math_sinf.c
	math_sinfv.c
	math_sinhf.c
	math_sqrtf.c
	math_sqrtfv.c
	math_tanf.c
	math_tanhf.c
	math_vec2.c
	math_vec3.c
	math_vec4.c
)

add_library(${PROJECT_NAME} STATIC ${LOCAL_MATH_NEON_SRC_LIST})
