#include "renderer/util/gl_contants.h"

namespace ark {

GLConstants::GLConstants()
{
    initConstants();
}

GLenum GLConstants::getEnum(const String &name)
{
    const auto iter = _constants.find(name);
    DCHECK(iter != _constants.end(), "Bad GLenum name \"%s\"", name.c_str());
    return iter->second;
}

GLenum GLConstants::getEnum(const String& name, GLenum defValue)
{
    const auto iter = _constants.find(name);
    return iter != _constants.end() ? iter->second : defValue;
}

void GLConstants::initConstants()
{
    _constants["nearest"] = GL_NEAREST;
    _constants["linear"] = GL_LINEAR;
    _constants["texture_mag_filter"] = GL_TEXTURE_MAG_FILTER;
    _constants["texture_min_filter"] = GL_TEXTURE_MIN_FILTER;
    _constants["texture_wrap_s"] = GL_TEXTURE_WRAP_S;
    _constants["texture_wrap_t"] = GL_TEXTURE_WRAP_T;
    _constants["texture_wrap_r"] = GL_TEXTURE_WRAP_R;
    _constants["clamp_to_edge"] = GL_CLAMP_TO_EDGE;
    _constants["clamp_to_border"] = GL_CLAMP_TO_BORDER;
    _constants["mirrored_repeat"] = GL_MIRRORED_REPEAT;
    _constants["repeat"] = GL_REPEAT;
    _constants["mirror_clamp_to_edge"] = GL_MIRROR_CLAMP_TO_EDGE;

    _constants["rgba"] = GL_RGBA;
    _constants["rgb"] = GL_RGB;
    _constants["alpha"] = GL_ALPHA;
    _constants["rg"] = GL_RG;
}

}
