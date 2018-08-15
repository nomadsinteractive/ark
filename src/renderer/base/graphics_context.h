#ifndef ARK_RENDERER_BASE_GRAPHICS_CONTEXT_H_
#define ARK_RENDERER_BASE_GRAPHICS_CONTEXT_H_

#include <map>
#include <stack>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_shader.h"
#include "renderer/inf/gl_resource.h"

namespace ark {

class ARK_API GraphicsContext {
public:
    GraphicsContext(const sp<GLContext>& glContext, const sp<GLResourceManager>& glResourceManager);
    ~GraphicsContext();

    void onSurfaceReady();
    void onDrawFrame();

    const sp<GLProgram>& getGLProgram(GLShader& shader);

    const sp<GLContext>& glContext() const;
    const sp<Camera>& camera() const;
    const sp<GLResourceManager>& glResourceManager() const;

    const sp<GLProgram>& program() const;

    void glUpdateMatrix(const String& name, const Matrix& matrix);

    void glUpdateMVPMatrix(const Matrix& matrix);
    void glUpdateVPMatrix(const Matrix& matrix);

    void glUseProgram(const sp<GLProgram>& program);

private:
    sp<GLContext> _gl_context;
    sp<GLResourceManager> _gl_resource_manager;

    sp<Variable<uint64_t>> _steady_clock;

    sp<GLProgram> _program;

    std::map<GLShader::Slot, WeakPtr<GLProgram>> _gl_prepared_programs;

    uint64_t _tick;

};

}

#endif
