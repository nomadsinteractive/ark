#ifndef ARK_RENDERER_BASE_GRAPHICS_CONTEXT_H_
#define ARK_RENDERER_BASE_GRAPHICS_CONTEXT_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_program.h"

namespace ark {

class ARK_API GraphicsContext {
public:
    GraphicsContext(const sp<GLContext>& glContext, const sp<GLResourceManager>& glResourceManager);
    ~GraphicsContext();

    void onSurfaceReady();
    void onDrawFrame();

    const sp<GLContext>& glContext() const;
    const sp<GLResourceManager>& glResourceManager() const;

    sp<GLProgram::Shader> makeShader(uint32_t version, GLenum type, const String& source);

    uint64_t tick() const;

private:
    sp<GLContext> _gl_context;
    sp<GLResourceManager> _gl_resource_manager;

    sp<Variable<uint64_t>> _steady_clock;
    std::unordered_map<GLenum, std::map<String, WeakPtr<GLProgram::Shader>>> _shaders;

    uint64_t _tick;

};

}

#endif
