#ifndef ARK_RENDERER_OPENGL_UTIL_GL_DEBUG_H_
#define ARK_RENDERER_OPENGL_UTIL_GL_DEBUG_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"
#include "core/util/strings.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"

#include "platform/gl/gl.h"
#include "platform/platform.h"

namespace ark {

class GLDebug {
public:
    static int32_t glGetBufferSize(GLenum target);
    static GLuint glTestIndexBuffer();
    static GLuint glTestArrayBuffer();

    template<typename T, typename P> static void glDumpBuffer(const P& buffer) {
        int32_t nBufferSize = glGetBufferSize(buffer.type());

        String data = glDumpBufferData<T>(buffer, nBufferSize);
        LOGD("buffer-id: %d; size: %d; data: [%s]", buffer.id(), nBufferSize, data.c_str());
    }

    template<typename T, typename P> static String glDumpBufferData(const P& buffer, uint32_t bufferSize) {
#ifndef ANDROID
        T* data = reinterpret_cast<T*>(glMapBuffer(buffer.type(), GL_READ_ONLY));
        String content = Strings::join<T>(data, 0, bufferSize / sizeof(T));
        glUnmapBuffer(buffer.type());
        return content;
#else
        T* data = reinterpret_cast<T*>(glMapBufferOES(buffer.type(), static_cast<GLenum>(GL_READ_ONLY)));
        String content = Strings::join<T>(data, 0, bufferSize / sizeof(T));
        glUnmapBufferOES(buffer.type());
        return "";
#endif
    }

    template<typename T> static void liBufferData(T* buffer, uint32_t offset, uint32_t length) {
        String content = Strings::join<T>(buffer, offset, length);
        LOGD("offset: %d; length: %d; data: [%s]", offset, length, content.c_str());
    }

    static void glPrintString(const char* name, GLenum value);
    static void glPrintInteger(const char* name, GLenum value);
    static void glCheckError(const char* tag);
    static void glDumpBitmapToPngFile(const Bitmap* bitmap, const char* filename);

};

}

#endif
