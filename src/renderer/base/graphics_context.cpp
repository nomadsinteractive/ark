#include "renderer/base/graphics_context.h"

#include "core/inf/variable.h"
#include "core/util/strings.h"

#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_program.h"
#include "renderer/base/render_engine.h"

#include "platform/platform.h"

namespace ark {

GraphicsContext::GraphicsContext(const sp<GLContext>& glContext, const sp<GLResourceManager>& glResources)
    : _gl_context(glContext), _gl_resource_manager(glResources), _steady_clock(Platform::getSteadyClock())
{
    _matrix_stack.push(MVPMatrix());
    _top = &_matrix_stack.top();
}

GraphicsContext::~GraphicsContext()
{
}

void GraphicsContext::onSurfaceReady()
{
    _gl_resource_manager->onSurfaceReady(*this);
}

void GraphicsContext::onDrawFrame()
{
    _tick = _steady_clock->val();
    _gl_resource_manager->onDrawFrame(*this);
}

const sp<GLProgram>& GraphicsContext::getGLProgram(GLShader& shader)
{
    if(shader.program())
    {
        if(shader.program()->id() == 0)
            shader.program()->prepare(*this);
        return shader.program();
    }

    const GLShader::Slot slot = shader.preprocess(*this);

    const auto iter = _gl_prepared_programs.find(slot);
    if(iter != _gl_prepared_programs.end())
    {
        const sp<GLProgram> program = iter->second.lock();
        if(program)
        {
            if(program->id() == 0)
                program->prepare(*this);
            shader.setProgram(program);
            return shader.program();
        }
        else
            _gl_prepared_programs.erase(iter);
    }

    const sp<GLProgram>& program = shader.makeGLProgram(*this);
    _gl_resource_manager->prepare(program, GLResourceManager::PS_ON_SURFACE_READY);
    program->prepare(*this);
    _gl_prepared_programs[slot] = program;
    return program;
}

const sp<GLResourceManager>& GraphicsContext::glResourceManager() const
{
    return _gl_resource_manager;
}

const sp<GLContext>& GraphicsContext::glContext() const
{
    return _gl_context;
}

void GraphicsContext::glOrtho(float left, float right, float top, float bottom, float near, float far)
{
    _top->setVP(Matrix::ortho(left, right, top, bottom, near, far));
}

void GraphicsContext::glUpdateMVPMatrix()
{
    const Matrix& mvp = _top->mvp();
    const GLProgram::Uniform& uniform = _program->getUniform("u_MVP");
    DCHECK(uniform, "Uniform u_MVP not found");
    uniform.setUniformMatrix4fv(1, GL_FALSE, mvp.value(), _tick);
}

void GraphicsContext::glUpdateVPMatrix()
{
    const Matrix& vp = _top->vp();
    const GLProgram::Uniform& uniform = _program->getUniform("u_VP");
    DCHECK(uniform, "Uniform u_VP not found");
    uniform.setUniformMatrix4fv(1, GL_FALSE, vp.value(), _tick);
}

void GraphicsContext::glUpdateModelMatrix()
{
    const GLProgram::Uniform& uniform = _program->getUniform("u_Model");
    DCHECK(uniform, "Uniform u_Model not found");
    const Matrix& model = _top->model();
    uniform.setUniformMatrix4fv(1, GL_FALSE, model.value(), _tick);
}

void GraphicsContext::glPushMatrix()
{
    _matrix_stack.push(MVPMatrix(*_top));
    _top = &_matrix_stack.top();
}

void GraphicsContext::glPopMatrix()
{
    DCHECK(_matrix_stack.size(), "Empty matrix stack");
    _matrix_stack.pop();
    _top = &_matrix_stack.top();
}

void GraphicsContext::glUseProgram(const sp<GLProgram>& program)
{
    DCHECK(program && program->id(), "Illegal program used");
    if(_program != program)
    {
        _program = program;
        program->use();
    }
}

const sp<GLProgram>& GraphicsContext::program() const
{
    return _program;
}

void GraphicsContext::MVPMatrix::setVP(const Matrix& vp)
{
    _vp = vp;
    _mvp = _vp * _model;
}

const Matrix& GraphicsContext::MVPMatrix::mvp() const
{
    return _mvp;
}

const Matrix& GraphicsContext::MVPMatrix::vp() const
{
    return _vp;
}

const Matrix& GraphicsContext::MVPMatrix::model() const
{
    return _model;
}

const Matrix& GraphicsContext::MVPMatrix::view() const
{
    return _view;
}

const Matrix& GraphicsContext::MVPMatrix::projection() const
{
    return _projection;
}

}
