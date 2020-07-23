#ifndef ARK_PLUGIN_ASSIMP_UTIL_ANIMATE_UTIL_H_
#define ARK_PLUGIN_ASSIMP_UTIL_ANIMATE_UTIL_H_

#include <functional>

#include <assimp/Importer.hpp>
#include <assimp/anim.h>
#include <assimp/scene.h>

#include "core/forwarding.h"

#include "assimp/forwarding.h"

namespace ark {
namespace plugin {
namespace assimp {

class AnimateUtil {
public:

    static aiMatrix4x4 interpolateTranslation(float time, const aiNodeAnim* pNodeAnim);
    // Returns a 4x4 matrix with interpolated rotation between current and next frame
    static aiMatrix4x4 interpolateRotation(float time, const aiNodeAnim* pNodeAnim);
    // Returns a 4x4 matrix with interpolated scaling between current and next frame
    static aiMatrix4x4 interpolateScale(float time, const aiNodeAnim* pNodeAnim);
    static const aiNodeAnim* findNodeAnim(const aiAnimation* animation, const String& nodeName);
};

}
}
}

#endif
