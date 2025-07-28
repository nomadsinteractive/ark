#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"
#include "core/util/strings.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"

#include "platform/gl/gl.h"
#include "platform/platform.h"

namespace ark::plugin::opengl {

class GLDebug {
public:
    static int32_t glGetBufferSize(GLenum target);

    template<typename T, typename P> static void glDumpBuffer(P& buffer, GLenum type) {
        int32_t nBufferSize = glGetBufferSize(type);

        String data = glDumpBufferData<T>(buffer, type, nBufferSize);
        LOGD("buffer-id: %d; size: %d; data: [%s]", static_cast<uint32_t>(buffer.id()), nBufferSize, data.c_str());
    }

    template<typename T, typename P> static String glDumpBufferData(P& buffer, GLenum type, uint32_t bufferSize) {
#ifndef ANDROID
        T* data = reinterpret_cast<T*>(glMapBuffer(type, GL_READ_ONLY));
        String content = Strings::join<T>(data, 0, bufferSize / sizeof(T));
        glUnmapBuffer(type);
        return content;
#else
        T* data = reinterpret_cast<T*>(glMapBufferOES(type, static_cast<GLenum>(GL_READ_ONLY)));
        String content = Strings::join<T>(data, 0, bufferSize / sizeof(T));
        glUnmapBufferOES(type);
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

};

}
