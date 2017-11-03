#ifndef ARK_RENDERER_BASE_GRAPHICS_CONTEXT_H_
#define ARK_RENDERER_BASE_GRAPHICS_CONTEXT_H_

#include <map>
#include <stack>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"

#include "graphics/base/matrix.h"

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

    const sp<GLResourceManager>& glResourceManager() const;
    const sp<GLContext>& glContext() const;

    const sp<GLProgram>& program() const;

    void glOrtho(float left, float right, float top, float bottom, float near, float far);
    void glUpdateMVPMatrix();
    void glUpdateMVMatrix();

    void glPushMatrix();
    void glPopMatrix();
    void translate(float x, float y, float z);
    void scale(float x, float y, float z);

    void glUseProgram(const sp<GLProgram>& program);

    const Matrix& projection() const;

private:
    class MVPMatrix {
    public:
        MVPMatrix();
        MVPMatrix(const MVPMatrix& other);

        void translate(float x, float y, float z);
        void scale(float x, float y, float z);

        void setProjection(const Matrix& projection);

        const Matrix& mvp(uint64_t tick);
        const Matrix& mv(uint64_t tick);

        const Matrix& projection() const;

        uint64_t lastModified() const;

    private:
        void update();

    private:
        Matrix _model;
        Matrix _view;
        Matrix _projection;

        Matrix _mvp;
        Matrix _mv;

        uint64_t _last_modified;
    };

private:
    sp<GLContext> _gl_context;
    sp<GLResourceManager> _gl_resource_manager;

    sp<Variable<uint64_t>> _steady_clock;

    sp<GLProgram> _program;

    std::map<GLShader::Slot, WeakPtr<GLProgram>> _gl_prepared_programs;
    std::stack<MVPMatrix> _matrix_stack;
    MVPMatrix* _top;

    uint64_t _tick;

};

}

#endif
