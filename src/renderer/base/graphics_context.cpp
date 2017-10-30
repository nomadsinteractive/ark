#include "renderer/base/graphics_context.h"

#include "core/util/strings.h"
#include "core/util/log.h"

#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_program.h"
#include "renderer/base/render_engine.h"

#include "platform/platform.h"

namespace ark {

GraphicsContext::GraphicsContext(const sp<GLResourceManager>& glResources)
    : _gl_resource_manager(glResources), _steady_clock(Platform::getSteadyClock())
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

    const auto iter = _gl_prepared_programs.find(shader.slot());
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

    const sp<GLProgram>& program = shader.makeGLProgram();
    _gl_resource_manager->prepare(program, GLResourceManager::PS_ON_SURFACE_READY);
    program->prepare(*this);
    _gl_prepared_programs[shader.slot()] = program;
    return program;
}

const sp<GLResourceManager>& GraphicsContext::glResourceManager() const
{
    return _gl_resource_manager;
}

void GraphicsContext::glOrtho(float left, float right, float top, float bottom, float near, float far)
{
    _top->setProjection(Matrix::ortho(left, right, top, bottom, near, far));
}

void GraphicsContext::glUpdateMVPMatrix()
{
    const Matrix& mvp = _top->mvp(_tick);
    const GLProgram::Uniform& uniform = _program->getUniform("u_MVPMatrix");
    uniform.setUniformMatrix4fv(1, GL_FALSE, mvp.value(), _tick);
}

void GraphicsContext::glUpdateMVMatrix()
{
    const Matrix& mv = _top->mv(_tick);
    const GLProgram::Uniform& uniform = _program->getUniform("u_MVMatrix");
    uniform.setUniformMatrix4fv(1, GL_FALSE, mv.value(), _tick);
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

void GraphicsContext::translate(float x, float y, float z)
{
    _top->translate(x, y, z);
}

void GraphicsContext::scale(float x, float y, float z)
{
    _top->scale(x, y, z);
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

const Matrix& GraphicsContext::projection() const
{
    return _top->projection();
}

GraphicsContext::MVPMatrix::MVPMatrix()
    : _last_modified(0)
{
}

GraphicsContext::MVPMatrix::MVPMatrix(const GraphicsContext::MVPMatrix& other)
    : _model(other._model), _view(other._view), _projection(other._projection), _mvp(other._mvp), _mv(other._mv), _last_modified(other._last_modified)
{
}

void GraphicsContext::MVPMatrix::translate(float x, float y, float z)
{
    _view.translate(x, y, z);
    _last_modified = 0;
}

void GraphicsContext::MVPMatrix::scale(float x, float y, float z)
{
    _view.scale(x, y, z);
    _last_modified = 0;
}

void GraphicsContext::MVPMatrix::setProjection(const Matrix& projection)
{
    _projection = projection;
    _last_modified = 0;
}

const Matrix& GraphicsContext::MVPMatrix::mvp(uint64_t tick)
{
    if(_last_modified < tick)
        update();
    return _mvp;
}

const Matrix& GraphicsContext::MVPMatrix::mv(uint64_t tick)
{
    if(_last_modified < tick)
        update();
    return _mv;
}

const Matrix& GraphicsContext::MVPMatrix::projection() const
{
    return _projection;
}

uint64_t GraphicsContext::MVPMatrix::lastModified() const
{
    return _last_modified;
}

void GraphicsContext::MVPMatrix::update()
{
    _mv = _view * _model;
    _mvp = _projection * _mv;
    _last_modified = 0;
}

}
