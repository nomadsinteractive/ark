#ifndef ARK_PLATFORMS_ANDROID_PLATFORM_GL_GL_H_
#define ARK_PLATFORMS_ANDROID_PLATFORM_GL_GL_H_

#if __ANDROID_API__ < 18
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#elif __ANDROID_API__ < 21
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#else
#include <GLES3/gl31.h>
#include <GLES3/gl3ext.h>
#endif

#include <GLES/gl.h>
#include <GLES/glext.h>

namespace ark {

static const GLenum GL_READ_ONLY = 0x88B8;

static const GLenum GL_CLAMP_TO_BORDER = 0x812D;
static const GLenum GL_MIRROR_CLAMP_TO_EDGE = 0x8743;
static const GLenum GL_R16 = 0x822A;
static const GLenum GL_R16_SNORM = 0x8F98;
static const GLenum GL_RG16 = 0x822C;
static const GLenum GL_RG16_SNORM = 0x8F99;
static const GLenum GL_RGB16 = 0x8054;
static const GLenum GL_RGB16_SNORM = 0x8F9A;
static const GLenum GL_RGBA16 = 0x805B;
static const GLenum GL_RGBA16_SNORM = 0x8F9B;

static const GLenum GL_POINT_SPRITE = 0x8861;
static const GLenum GL_PROGRAM_POINT_SIZE = 0x8642;

}

#endif
