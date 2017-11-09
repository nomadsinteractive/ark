#ifndef ARK_RENDERER_BASE_GL_SHADER_SOURCE_H_
#define ARK_RENDERER_BASE_GL_SHADER_SOURCE_H_

#include <map>

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/collection/list.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_attribute.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/gl_shader_preprocessor.h"

namespace ark {

class ARK_API GLShaderSource {
public:
    GLShaderSource(const String& vertex, const String& fragment, const sp<RenderController>& renderController);

    void loadPredefinedParam(BeanFactory& factory, const sp<Scope>& args, const document& manifest);

    sp<GLProgram> makeGLProgram(GraphicsContext& graphicsContext) const;

    void addPredefinedAttribute(const String& name, const String& type, uint32_t scopes = 0);
    void addUniform(const String& name, GLUniform::Type type, const sp<Flatable>& flatable, const sp<Changed>& changed);

    const sp<GLSnippet>& snippet() const;
    void addSnippet(const sp<GLSnippet>& snippet);

    GLShader::Slot preprocess(GraphicsContext& graphicsContext);

    GLShaderPreprocessor& vertex();
    GLShaderPreprocessor& fragment();

private:
    void initialize();

    void addAttribute(const String& name, const String& type);
    void insertPredefinedUniforms(const String& source, StringBuilder& sb);

    void loadPredefinedAttribute(const document& manifest);
    GLAttribute getPredefinedAttribute(const String& name, const String& type);

    void loadPredefinedUniform(BeanFactory& factory, const sp<Scope>& args, const document& manifest);

    op<GLShaderPreprocessorContext> _preprocessor_context;

    GLShaderPreprocessor _vertex;
    GLShaderPreprocessor _fragment;
    uint32_t _stride;
    sp<RenderController> _render_controller;

    std::map<String, GLAttribute> _attributes;
    List<GLUniform> _uniforms;

    sp<GLSnippet> _snippet;

    friend class GLShader;
    friend class GLShaderPreprocessorContext;

};

}

#endif
