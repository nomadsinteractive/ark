#include "renderer/base/graphics_context.h"

#include "core/inf/variable.h"
#include "core/util/strings.h"

#include "graphics/base/camera.h"

#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_program.h"
#include "renderer/base/render_engine.h"

#include "platform/platform.h"

namespace ark {

GraphicsContext::GraphicsContext(const sp<GLContext>& glContext, const sp<GLResourceManager>& glResources)
    : _gl_context(glContext), _gl_resource_manager(glResources), _steady_clock(Platform::getSteadyClock())
{
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

void GraphicsContext::glUpdateMVPMatrix(const Matrix& matrix)
{
    glUpdateMatrix("u_MVP", matrix);
}

void GraphicsContext::glUpdateVPMatrix(const Matrix& matrix)
{
    glUpdateMatrix("u_VP", matrix);
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

void GraphicsContext::glUpdateMatrix(const String& name, const Matrix& matrix)
{
    const GLProgram::Uniform& uniform = _program->getUniform(name);
    DCHECK(uniform, "Uniform %s not found", name.c_str());
    uniform.setUniformMatrix4fv(1, GL_FALSE, matrix.value(), _tick);
}

}
