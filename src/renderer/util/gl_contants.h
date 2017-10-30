#ifndef ARK_RENDERER_UTIL_GL_CONTANTS_H_
#define ARK_RENDERER_UTIL_GL_CONTANTS_H_

#include <map>

#include "core/base/string.h"

#include "platform/gl/gl.h"

namespace ark {

class GLConstants {
public:
    GLConstants();

    GLenum getEnum(const String& name);
    GLenum getEnum(const String& name, GLenum defValue);

private:
    void initConstants();

private:
    std::map<String, GLenum> _constants;
};

}

#endif
